#pragma once
#include <iostream>
#include <cstdint>

enum class PacketType : uint8_t {CONTROL = 1, CHAT = 2, AUDIO = 3}; // store as 1 byte

// 1 handle call, 2 show the text, 3 play the sound