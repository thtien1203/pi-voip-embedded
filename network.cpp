#include <iostream>
#include "network.hpp"
#include "call.hpp"

int createSocket() {
    int socketfd = socket(AF_INET, SOCK_DGRAM, 0);  // socket's id
    if (socketfd < 0) {
        std::cerr << "Failed to create socket\n";
        return false;
    }
    return socketfd;
}

bool receiverAddress(sockaddr_in &myAddr, int socketfd, int my_port) {
    myAddr.sin_family = AF_INET;
    myAddr.sin_port = htons(my_port);  // put number's bytes in the order the network expects
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY); // accept messages arrving on any of my address

    // connect socket to specific port (5000) on computer
    int bindResult = bind(socketfd, reinterpret_cast<sockaddr*>(&myAddr), sizeof(myAddr));
    if (bindResult == -1) {
        perror("bind");
        return false;
    }
    return true;
}

bool senderAddress(sockaddr_in &receiveAddr, const std::string &ip_address, int other_port) {
    receiveAddr.sin_family = AF_INET;
    receiveAddr.sin_port = htons(other_port);
    int result = inet_pton(AF_INET, ip_address.c_str(), &(receiveAddr.sin_addr));    // convert ip address from text into number format(binary), return status number, sucess or failed
    if (result != 1) {
        std::cerr << "Invalid IP address format\n";
        return false;
    }
    return true;
}

bool sendMessage(int socketfd, const sockaddr_in& receiveAddr, const std::string& message) {
    //std::getline(std::cin, message);
    // socket id, message, message length, 0, cast address, size 
    ssize_t send_m = sendto(socketfd, message.c_str(), message.size(), 0, reinterpret_cast<const sockaddr*>(&receiveAddr), sizeof(receiveAddr));
    if (send_m == -1) {
        std::cerr << "Failed to send the message\n";
        return false;
        // perror("sendto");
    }
    return true;
}
void receiveLoop (int socketfd) {
    char buffer[1024];
    sockaddr_in senderAddr {};
    socklen_t senderLen = sizeof(senderAddr);

    // wait for the message and print
    while (true) {
        // receive from (who sent it) (how many bytes it wrote)
        senderLen = sizeof(senderAddr);
        ssize_t byteReceive = recvfrom(socketfd, buffer, sizeof(buffer)-1, 0, reinterpret_cast<sockaddr*>(&senderAddr), &senderLen);
        if (byteReceive == -1) {
            perror("recvfrom");
            continue;
        }

        buffer[byteReceive] = '\0';     // FIX LATER WHEN DO AUDIO BECAUSE IT IS NOT A TEXT
        handleIncoming(std::string(buffer));
        // // MIGHT DELETE THIS PART LATER
        // // who sent it
        // char senderIP[INET_ADDRSTRLEN];          // space for an IP as text, like "127.0.0.1"
        // inet_ntop(AF_INET, &(senderAddr.sin_addr), senderIP, sizeof(senderIP));
        // int senderPort = ntohs(senderAddr.sin_port);
        // std::cout << "From " << senderIP << ":" << senderPort
        //         << " -> " << buffer << "\n";
    }
}