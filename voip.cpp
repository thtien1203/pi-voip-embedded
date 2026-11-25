#include <iostream>
#include <stdexcept>
#include <thread>
#include "network.hpp"
#include "call.hpp"
#include "audio.hpp"
#include "codec.hpp"
#include "hardware.hpp"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <my_port> <other's IP_address> <other's port_number>\n";
        return 1;
    }
    
    int my_port = 0;
    std::string ip_address = argv[2];
    int other_port = 0;
    try {
        my_port = std::stoi(argv[1]);
        other_port = std::stoi(argv[3]);
    } 
    catch (const std::invalid_argument &e) {
        std::cerr << "Error. Port must be a valid number\n";
        return 1;
    }
    catch (const std::out_of_range &e) {
        std::cerr << "Error. Port number is too large\n";
        return 1;
    }

    if (my_port < 1024 || my_port > 65535 || other_port < 1024 || other_port > 65535) {
        std::cerr << "Port number must be in range 1024 to 65535\n";
        return 1; 
    }
    // CREATE UDP SOCKET
    int socketfd = createSocket();  // socket's id
    if (socketfd < 0) {
        return 1;
    }

    // Fill receiver address (THIS IS RECEIVER, RECEIVER NEEDS RECEIVER'S PORT)
    sockaddr_in myAddr {};
    if (!receiverAddress(myAddr, socketfd, my_port)) {
        close(socketfd);
        return 1;
    }

    sockaddr_in receiveAddr {};
    if (!senderAddress(receiveAddr, ip_address, other_port)) {
        close(socketfd);
        return 1;
    }

    // rasp pi 5
    if (!hardwareInit()) {
        close(socketfd);
        return 1;
    } 

    if (!audioInit()) { // start audio
        close(socketfd);
        return 1;
    }
    if (!audioStart()) {
        audioTerminate();
        close(socketfd);
        return 1;
    }

    // create encoder and decoder
    if (!codecInit()) {
        audioStop();
        audioTerminate();
        close(socketfd);
        return 1;
    }
    std::thread playbackThread(playbackLoop);
    std::thread audioThread(audioSendLoop, socketfd, receiveAddr);

    std::thread t1(receiveLoop, socketfd);
    t1.detach();    // let this thread runs on this own

    // fill the address for receiver    (THIS IS SENDER, SENDER NEEDS RECEIVER'S IP ADDRESS AND PORT)
                                        // SENDER -> RECEIVER, RECEIVER -> SENDER
                                    
    std::cout << "Address converted successfully\n";
    while (true) {
        std::string message;
        std::getline(std::cin, message);
        if (message == "quit" || message == "q") {
            if (callState != CallState::IDLE) {
                handleCommand(socketfd, receiveAddr, "hangup");  // show hangup both sides
            }
            break;
        }
        if (message == "b") {
            onButtonPressed(socketfd, receiveAddr);
            continue;
        }
        handleCommand(socketfd, receiveAddr, message);
    }
    running = false;    // tell audio and playback thread stop
    audioThread.join();
    playbackThread.join();
    codecDestroy();
    audioStop();
    audioTerminate();
    setLed(false);
    hardwareCleanup();
    close(socketfd);
    return 0;
}
