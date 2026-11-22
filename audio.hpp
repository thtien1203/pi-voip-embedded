#pragma once
#include <iostream>
#include <cstdint>


#define SAMPLE_RATE 48000
#define CHANNELS 1
#define FRAMES_PER_BUFFER 960

constexpr size_t AUDIO_BYTES     = FRAMES_PER_BUFFER * sizeof(int16_t);   // 1920
constexpr size_t MAX_CHAT_BYTES  = 1024;
constexpr size_t MAX_PACKET_SIZE = 1 + std::max(AUDIO_BYTES, MAX_CHAT_BYTES);

bool audioInit ();
bool audioStart();
bool readChunk(int16_t *buffer);
bool writeChunk(int16_t *buffer);
void audioStop();
void audioTerminate();