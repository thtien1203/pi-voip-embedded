#include <iostream>
#include <vector>
#include <cstring>
#include "network.hpp"
#include "call.hpp"
#include "packet.hpp"
#include "audio.hpp"
#include <chrono>

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

bool sendMessage(PacketType type, int socketfd, const sockaddr_in& receiveAddr, const std::string& message) {

    return sendPacket(socketfd, receiveAddr, type, message.data(), message.size());
}

void receiveLoop (int socketfd) {
    uint8_t buffer[MAX_PACKET_SIZE];
    sockaddr_in senderAddr {};
    socklen_t senderLen = sizeof(senderAddr);

    // wait for the message and print
    while (true) {
        // receive from (who sent it) (how many bytes it wrote)
        senderLen = sizeof(senderAddr);
        ssize_t byteReceive = recvfrom(socketfd, buffer, sizeof(buffer), 0, reinterpret_cast<sockaddr*>(&senderAddr), &senderLen);
        if (byteReceive == -1) {
            perror("recvfrom");
            continue;
        }
        if (byteReceive < 1) {
            continue;
        }

        PacketType type = static_cast<PacketType>(buffer[0]);
        if (type == PacketType::CONTROL || type == PacketType::CHAT) {
            std::string text(reinterpret_cast<const char *>(buffer + 1), byteReceive - 1);
            handleIncoming(type, text);
        }
        else if (type == PacketType::AUDIO) {
            if (callState != CallState::CONNECTED) {
                continue;
            }
            if (static_cast<size_t>(byteReceive - 1) != AUDIO_BYTES) {
                continue;
            }
        
            int16_t samples[FRAMES_PER_BUFFER];
            std::memcpy(samples, buffer + 1, AUDIO_BYTES);
            writeChunk(samples);    // play it

        }
    }
}

bool sendPacket(int socketfd, const sockaddr_in& receiveAddr, PacketType type, const void* data, size_t dataSize) {  // add a label to my data and sends it to other person
    std::vector<uint8_t> buffer(1 + dataSize);
    buffer[0] = static_cast<uint8_t>(type);
    std::memcpy(buffer.data() + 1, data, dataSize);  // copy data(copy to, copy from, size)
    ssize_t send_m = sendto(socketfd, buffer.data(), buffer.size(), 0, reinterpret_cast<const sockaddr*>(&receiveAddr), sizeof(receiveAddr));
    if (send_m == -1) {
        perror("sendto");
        return false;
    }
    return true;
}