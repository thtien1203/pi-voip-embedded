#ifndef CALL_HPP
#define CALL_HPP
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <atomic>

enum class CallState {IDLE, CALLING, RINGING, CONNECTED};
extern std::atomic<CallState> callState;
void handleIncoming(const std::string &message);
void handleCommand(int socketfd, const sockaddr_in& receiveAddr, const std::string& text);
#endif