#include <iostream>
#include <sys/socket.h> 
#include <netinet/in.h> // the address structure for IPv4
#include <arpa/inet.h>
#include <unistd.h>     // close socket at the end
#include <stdexcept>


int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <IP_address> <port_number>\n";
        return 1;
    }
    
    std::string ip_address = argv[1];
    int port = 0;
    try {
        port = std::stoi(argv[2]);
    } 
    catch (const std::invalid_argument &e) {
        std::cerr << "Error. Port must be a valid number\n";
        return 1;
    }

    // create udp socket
    int socketfd = socket(AF_INET, SOCK_DGRAM, 0);  // socket's id
    if (socketfd < 0) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    // fill the address for receiver
    sockaddr_in receiveAddr {};
    receiveAddr.sin_family = AF_INET;
    receiveAddr.sin_port = htons(port);
    int result = inet_pton(AF_INET, ip_address.c_str(), &(receiveAddr.sin_addr));    // convert ip address from text into number format(binary), return status number, sucess or failed
    if (result == 1) {
        std::cout << "Address converted successfully\n";
    }
    else {
        std::cerr << "Invalid IP address format\n";
        return 1;
    }

    // wait for you to type message and senD
    std::string message = "hello";
    // socket id, message, message length, 0, cast address, size 
    ssize_t send_m = sendto(socketfd, message.c_str(), message.size(), 0, reinterpret_cast<sockaddr*>(&receiveAddr), sizeof(receiveAddr));
    if (send_m == -1) {
        // std::cerr << "Failed to send the message\n";
        // return 1;
        perror("sendto");
    }
    close(socketfd);
    return 0;
}