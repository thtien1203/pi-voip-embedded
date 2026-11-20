#include <iostream>
#include <sys/socket.h> 
#include <netinet/in.h> // the address structure for IPv4
#include <arpa/inet.h>
#include <unistd.h>     // close socket at the end
#include <stdexcept>
#include <thread>

void receiveLoop (int socketfd); 

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
    int socketfd = socket(AF_INET, SOCK_DGRAM, 0);  // socket's id
    if (socketfd < 0) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }


    // Fill receiver address (THIS IS RECEIVER, RECEIVER NEEDS RECEIVER'S PORT)
    sockaddr_in myAddr {};
    myAddr.sin_family = AF_INET;
    myAddr.sin_port = htons(my_port);  // put number's bytes in the order the network expects
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY); // accept messages arrving on any of my address

    // connect socket to specific port (5000) on computer
    int bindResult = bind(socketfd, reinterpret_cast<sockaddr*>(&myAddr), sizeof(myAddr));
    if (bindResult == -1) {
        perror("bind");
        close(socketfd);
        return 1;
    }

    std::thread t1(receiveLoop, socketfd);
    t1.detach();    // let this thread runs on this own

    // fill the address for receiver    (THIS IS SENDER, SENDER NEEDS RECEIVER'S IP ADDRESS AND PORT)
                                        // SENDER -> RECEIVER, RECEIVER -> SENDER
    sockaddr_in receiveAddr {};
    receiveAddr.sin_family = AF_INET;
    receiveAddr.sin_port = htons(other_port);
    int result = inet_pton(AF_INET, ip_address.c_str(), &(receiveAddr.sin_addr));    // convert ip address from text into number format(binary), return status number, sucess or failed
    if (result == 1) {
        std::cout << "Address converted successfully\n";
    }
    else {
        std::cerr << "Invalid IP address format\n";
        close(socketfd);
        return 1;
    }
    while (true) {
        std::string message;
        std::getline(std::cin, message);
        // socket id, message, message length, 0, cast address, size 
        ssize_t send_m = sendto(socketfd, message.c_str(), message.size(), 0, reinterpret_cast<sockaddr*>(&receiveAddr), sizeof(receiveAddr));
        if (send_m == -1) {
            // std::cerr << "Failed to send the message\n";
            // return 1;
            perror("sendto");
        }
    }

    close(socketfd);
    return 0;
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
        // who sent it
        char senderIP[INET_ADDRSTRLEN];          // space for an IP as text, like "127.0.0.1"
        inet_ntop(AF_INET, &(senderAddr.sin_addr), senderIP, sizeof(senderIP));
        int senderPort = ntohs(senderAddr.sin_port);
        std::cout << "From " << senderIP << ":" << senderPort
                << " -> " << buffer << "\n";
    }
}