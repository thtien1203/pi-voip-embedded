#include <iostream>
#include <cstdint>
#include "audio.hpp"

int main() {
    // 1. Start PortAudio
    if (!audioInit()) {
        std::cerr << "audioInit failed\n";
        return 1;
    }

    // 2. Open and start the mic and speaker streams
    if (!audioStart()) {
        std::cerr << "audioStart failed\n";
        audioTerminate();
        return 1;
    }

    // 3. One small buffer for one 20 ms chunk
    int16_t buffer[FRAMES_PER_BUFFER];

    // 4. Loopback for 10 seconds: read from mic -> play to speakers
    const int numChunks = SAMPLE_RATE * 10 / FRAMES_PER_BUFFER;   // 500
    std::cout << "Loopback for 10 seconds... put on headphones and talk\n";

    for (int i = 0; i < numChunks; i++) {
        if (!readChunk(buffer)) {
            std::cerr << "readChunk failed\n";
            break;
        }
        if (!writeChunk(buffer)) {
            std::cerr << "writeChunk failed\n";
            break;
        }
    }

    // 5. Clean up
    audioStop();
    audioTerminate();
    std::cout << "Done\n";
    return 0;
}