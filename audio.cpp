#include <iostream>
#include "audio.hpp"
#include <portaudio.h>

static bool checkError(PaError error) {
    if (error == paNoError) {
        return true;
    }
    std::cerr << "Port Audio error: " << Pa_GetErrorText(error) << "\n";
    return false;
}

static PaStream * inputStream = nullptr;
static PaStream * outputStream = nullptr;

bool audioInit() {
    PaError error = Pa_Initialize();
    if (!checkError(error)) {   // if failse
        return false;
    }
    return true;
}

bool audioStart() {
    // handles both mic (input) and speakers (output)
    // open the mic
    PaError err = Pa_OpenDefaultStream(&inputStream, CHANNELS, 0, paInt16, SAMPLE_RATE, FRAMES_PER_BUFFER, nullptr, nullptr);
    if (!checkError(err)) {
        inputStream = nullptr;
        return false;
    }

    err = Pa_OpenDefaultStream(&outputStream, 0, CHANNELS, paInt16, SAMPLE_RATE, FRAMES_PER_BUFFER, nullptr, nullptr);
    if (!checkError(err)) {
        Pa_CloseStream(inputStream);
        inputStream = nullptr;
        outputStream = nullptr;
        return false;
    }

    // start the mic
    err = Pa_StartStream(inputStream);   
    if (!checkError(err)) {
        Pa_CloseStream(inputStream);
        Pa_CloseStream(outputStream);
        inputStream = nullptr;
        outputStream = nullptr;
        return false;
    }

    err = Pa_StartStream(outputStream);   
    if (!checkError(err)) {
        Pa_StopStream(inputStream);    // mic is running, stop it andd close both
        Pa_CloseStream(inputStream);
        Pa_CloseStream(outputStream);
        inputStream = nullptr;
        outputStream = nullptr;
        return false;
    }

    // int16_t silence[FRAMES_PER_BUFFER] = {};      // all zeros = silence
    // for (int i = 0; i < 3; i++) {
    //     Pa_WriteStream(outputStream, silence, FRAMES_PER_BUFFER);
    // }
    return true;
}

bool readChunk(int16_t *buffer) {
    PaError err = Pa_ReadStream(inputStream, buffer, FRAMES_PER_BUFFER);
    if (err == paInputOverflowed) {
        return true;
    }
    return checkError(err);
}

bool writeChunk(int16_t *buffer) {
    PaError err = Pa_WriteStream(outputStream, buffer, FRAMES_PER_BUFFER);
    if (err == paOutputUnderflowed) {
        return true;
    }
    return checkError(err);
}

void audioStop() {
    Pa_StopStream(inputStream);
    Pa_StopStream(outputStream);
    Pa_CloseStream(inputStream);
    Pa_CloseStream(outputStream);
    inputStream = nullptr;
    outputStream = nullptr;
}

void audioTerminate() {
    Pa_Terminate();
}