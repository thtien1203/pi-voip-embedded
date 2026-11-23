#pragma once
#include <cstdint>

void jitterPush(const int16_t *chunk);
bool jitterPop(int16_t *output);
void jitterClear();