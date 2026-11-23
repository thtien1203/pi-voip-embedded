#pragma once
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <atomic>
#include "packet.hpp"
#include "jitter.hpp"

enum class CallState {IDLE, CALLING, RINGING, CONNECTED};
extern std::atomic<CallState> callState;
extern std::atomic<bool> running;
void handleIncoming(PacketType type, const std::string &message);
void handleCommand(int socketfd, const sockaddr_in& receiveAddr, const std::string& text);
void audioSendLoop(int socketfd, sockaddr_in receiveAddr);
void playbackLoop();