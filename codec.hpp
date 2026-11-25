#pragma once
#include <cstdint>

constexpr int MAX_OPUS_BYTES = 1275;

bool codecInit();
int encodeChunk(const int16_t* pcm_in, uint8_t* out, int maxBytes);    // opus encode 1920 bytes -> 60 bytes
bool decodeChunk(const uint8_t* data, int size, int16_t* pcm_out);      // opus decode 60 bytes -> 1920 bytes
void codecDestroy();
