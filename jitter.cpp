#include <deque>
#include <array>
#include <mutex>
#include <algorithm>
#include "jitter.hpp"
#include "audio.hpp"

static constexpr size_t MAX_JITTER_CHUNKS = 5;
static std::deque<std::array<int16_t, FRAMES_PER_BUFFER>> jitterQueue;
static std::mutex jitterMutex;

void jitterPush(const int16_t *chunk) { // add a chunk to the back of the line, the receive thread calls it every time an audio packet arrives
    std::lock_guard<std::mutex> lock(jitterMutex);
    if (jitterQueue.size() >= MAX_JITTER_CHUNKS) {  
        jitterQueue.pop_front();    // drop the oldest if full
    }
    std::array<int16_t, FRAMES_PER_BUFFER> newChunk;
    std::copy(chunk, chunk + FRAMES_PER_BUFFER, newChunk.begin());  // copy into new array
    jitterQueue.push_back(newChunk);
}

bool jitterPop(int16_t *output) {   // take a chunk from the front of the line, the playback
    std::lock_guard<std::mutex> lock(jitterMutex);
    if (jitterQueue.empty()) {  // nothing to play
        return false;
    }
    std::copy(jitterQueue.front().begin(), jitterQueue.front().end(), output);   // start, end, where
    jitterQueue.pop_front();
    return true;
}

void jitterClear() {
    std::lock_guard<std::mutex> lock(jitterMutex);
    jitterQueue.clear();
}