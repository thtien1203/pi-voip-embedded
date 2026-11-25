#include "call.hpp"
#include <iostream>
#include <algorithm>
#include "network.hpp"
#include "audio.hpp"
#include "jitter.hpp"
#include "codec.hpp"
#include "hardware.hpp"

std::atomic<CallState> callState{CallState::IDLE};
std::atomic<bool> running{true};
static void setState(CallState newState) { // change the call state and update LED at the same time
    callState = newState;
    setLed(newState == CallState::CONNECTED);
}
void handleIncoming(PacketType type, const std::string &message) {
    // handle message arrives 
    //std::cout << "DEBUG received: " << message << "\n";
    if (type == PacketType::CHAT) {
        if (callState == CallState::CONNECTED) {
            std::cout << "Friend: " << message << std::endl;
        }
    }
    else if (type == PacketType::CONTROL) {
        if (message == "CALL") {    
            if (callState == CallState::IDLE) {
                // callState = CallState::RINGING;
                setState(CallState::RINGING);
                std::cout << "Incoming call...type accept\n";
            }
        }
        else if (message == "ACCEPT") {      
            if (callState == CallState::CALLING) {
                //callState = CallState::CONNECTED;
                setState(CallState::CONNECTED);
                std::cout << "Call connected\n";
            }
        }
        else if (message == "HANGUP") { // handup handles 3 arrives: calling, connected, ringing
            if (callState != CallState::IDLE) {
                // callState = CallState::IDLE;
                setState(CallState::IDLE);
                std::cout << "Call ended\n";
                jitterClear();
            }
        }
    }

}

void handleCommand(int socketfd, const sockaddr_in& receiveAddr, const std::string& text) {
    // handle message type 
    if (text == "call") {
        if (callState == CallState::IDLE) {
            if (sendMessage(PacketType::CONTROL, socketfd, receiveAddr, "CALL")) {   // did my message arrive? if i want to send call
                // callState = CallState::CALLING;
                setState(CallState::CALLING);
                std::cout << "Calling...\n";
            }
        }
        else {
            std::cout << "Already in a call\n"; 
        }
    }
    else if (text == "accept") {
        if (callState == CallState::RINGING) {
            if (sendMessage(PacketType::CONTROL, socketfd, receiveAddr, "ACCEPT")) {   
                //callState = CallState::CONNECTED;
                setState(CallState::CONNECTED);
                std::cout << "Call connected\n";
            } 
        }
        else {
            std::cout << "No incoming call\n"; 
        }
    }
    else if (text == "hangup") {
        if (callState != CallState::IDLE) {
            if (sendMessage(PacketType::CONTROL, socketfd, receiveAddr, "HANGUP")) {   
                // callState = CallState::IDLE;
                setState(CallState::IDLE);
                std::cout << "Call ended\n";
                jitterClear();
            }  
        }
        else {
            std::cout << "Not in a call\n"; 
        }
    }
    else {
        if (callState != CallState::CONNECTED) {
            std::cout << "Not in a call. Type call to start one\n";
        }
        else if (text.size() > MAX_CHAT_BYTES) {
            std::cout << "Message too long\n";
        }
        else {
            sendMessage(PacketType::CHAT, socketfd, receiveAddr, text);
        }
    }
}

void onButtonPressed(int socketfd, const sockaddr_in& receiveAddr) {
    CallState state = callState;
    if (state == CallState::IDLE) {
        handleCommand(socketfd, receiveAddr, "call");
    }
    else if (state == CallState::RINGING) {
        handleCommand(socketfd, receiveAddr, "accept");
    }
    else {
        handleCommand(socketfd, receiveAddr, "hangup");
    }
}

void audioSendLoop(int socketfd, sockaddr_in receiveAddr) {
    int16_t buffer[FRAMES_PER_BUFFER];
    uint8_t encoded[MAX_OPUS_BYTES];    // buffer byte compress audio

    while(running) {
        if (!readChunk(buffer)) {
            continue;
        }
        if (callState == CallState::CONNECTED) {
            int bytes = encodeChunk(buffer, encoded, MAX_OPUS_BYTES);   // compress
            if (bytes < 0) {
                continue;
            }
            sendPacket(socketfd, receiveAddr, PacketType::AUDIO, encoded, bytes); // send
        }

    }

}

void playbackLoop() {
    int16_t chunk[FRAMES_PER_BUFFER];
    while (running) {
        bool hasAudio = (callState == CallState::CONNECTED) && jitterPop(chunk);
        if (!hasAudio) {
            // no call, or queue empty, just play silence
            std::fill(chunk, chunk + FRAMES_PER_BUFFER, 0);
        }

        writeChunk(chunk);
    }
}