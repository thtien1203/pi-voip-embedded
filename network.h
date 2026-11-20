#include <sys/socket.h> 
#include <netinet/in.h> // the address structure for IPv4
#include <arpa/inet.h>
#include <unistd.h>     // close socket at the end
#include <iostream>
#include <string>

#ifndef NETWORK_H
#define NETWORK_H

int createSocket();
bool receiverAddress(sockaddr_in &myAddr, int socketfd, int my_port);
bool senderAddress(sockaddr_in &receiveAddr, const std::string &ip_address, int other_port);
bool sendMessage(int socketfd, const sockaddr_in& peerAddr, const std::string& message);
void receiveLoop (int socketfd); 
#endif