#include <iostream>
#include <cstdint>
#include <portaudio.h>

#define SAMPLE_RATE 48000
#define CHANNELS 1
#define FRAMES_PER_BUFFER 960

bool checkError(PaError error) {
    if (error == paNoError) {
        return true;
    }
    std::cerr << "Port Audio error:" << Pa_GetErrorText(error) << "\n";
    return false;
}

int main() {
    PaError error = Pa_Initialize();
    if (!checkError(error)) {   // if failse
        return 1;
    }
    PaStream * stream = nullptr;
    // 1 stream handles both mic and speakers
    PaError errorOpenStream = Pa_OpenDefaultStream(&stream, CHANNELS, CHANNELS, paInt16, SAMPLE_RATE, FRAMES_PER_BUFFER, nullptr, nullptr);   // set up the pipe from mic to program
    if (!checkError(errorOpenStream)) {
        Pa_Terminate();
        return 1;
    }

    // start stream
    PaError startStreamErr = Pa_StartStream(stream);   
    if (!checkError(startStreamErr)) {
        Pa_CloseStream(stream);
        Pa_Terminate();
        return 1;
    }

    int16_t chunk[FRAMES_PER_BUFFER];
    int numChunk = SAMPLE_RATE * 10 / FRAMES_PER_BUFFER;
    std::cout << "Loopback for 10 seconds... put on headphones and talk\n";
    for (int i = 0; i < numChunk; i++) {
        PaError readStream = Pa_ReadStream(stream, chunk, FRAMES_PER_BUFFER);
        if (readStream == paInputOverflowed) {
            std::cout << "Overflow\n";
        }
        else if (readStream != paNoError) {
            std::cout << checkError(readStream);
            break;
        }

        PaError writeStream = Pa_WriteStream(stream, chunk, FRAMES_PER_BUFFER);    // where this chunk start inside record
        if (writeStream == paOutputUnderflowed) {
            std::cout << "Underflow\n";
        }
        else if (writeStream != paNoError) {
            std::cout << checkError(writeStream);
            break;
        }
    }
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    return 0;
}