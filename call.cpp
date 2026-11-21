#include "call.hpp"
#include <iostream>
#include "network.hpp"

std::atomic<CallState> callState{CallState::IDLE};

void handleIncoming(const std::string &message) {
    // handle message arrives 
    //std::cout << "DEBUG received: " << message << "\n";
    if (message == "CALL") {    
        if (callState == CallState::IDLE) {
            callState = CallState::RINGING;
            std::cout << "Incoming call...type accept\n";
        }
    }
    else if (message == "ACCEPT") {      
        if (callState == CallState::CALLING) {
            callState = CallState::CONNECTED;
            std::cout << "Call connected\n";
        }
    }
    else if (message == "HANGUP") { // handup handles 3 arrives: calling, connected, ringing
        if (callState != CallState::IDLE) {
            callState = CallState::IDLE;
            std::cout << "Call ended\n";
        }
    }
    else {
        if (callState == CallState::CONNECTED) {
            std::cout << "Chat message: " << message << "\n";
        }
    }

}

void handleCommand(int socketfd, const sockaddr_in& receiveAddr, const std::string& text) {
    // handle message type 
    if (text == "call") {
        if (callState == CallState::IDLE) {
            if (sendMessage(socketfd, receiveAddr, "CALL")) {   // did my message arrive? if i want to send call
                callState = CallState::CALLING;
                std::cout << "Calling...\n";
            }
        }
        else {
            std::cout << "Already in a call\n"; 
        }
    }
    else if (text == "accept") {
        if (callState == CallState::RINGING) {
            if (sendMessage(socketfd, receiveAddr, "ACCEPT")) {   
                std::cout << "Call connected\n";
            } 
        }
        else {
            std::cout << "No incoming call\n"; 
        }
    }
    else if (text == "hangup") {
        if (callState != CallState::IDLE) {
            if (sendMessage(socketfd, receiveAddr, "HANGUP")) {   
                std::cout << "Call ended\n";
            }  
        }
        else {
            std::cout << "Not in a call\n"; 
        }
    }
    else {
        if (callState == CallState::CONNECTED) {
            sendMessage(socketfd, receiveAddr, text);
        }
        else {
            std::cout << "Not in a call. Type call to start one\n";
        }
    }
}