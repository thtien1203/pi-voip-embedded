#include <iostream>
#include <sys/socket.h> 
#include <netinet/in.h> // the address structure for IPv4
#include <arpa/inet.h>
#include <unistd.h>     // close socket at the end
#include <stdexcept>

int main(int argc, char *argv[]) {

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port_number>\n";
        return 1;
    }

    int port = 0;
    try {
        port = std::stoi(argv[1]);
    } 
    catch (const std::invalid_argument &e) {
        std::cerr << "Error. Port must be a valid number\n";
        return 1;
    }

    // create socket
    int socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketfd < 0) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    // fill receiver address
    sockaddr_in receiverAddr {};
    receiverAddr.sin_family = AF_INET;
    receiverAddr.sin_port = htons(port);  // put number's bytes in the order the network expects
    receiverAddr.sin_addr.s_addr = htonl(INADDR_ANY); // accept messages arrving on any of my address

    // connect socket to specific port (5000) on computer
    int _bind = bind(socketfd, reinterpret_cast<sockaddr*>(&receiverAddr), sizeof(receiverAddr));
    if (_bind == -1) {
        perror("bind");
    }
     
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

    
    close(socketfd);
    return 0;
}