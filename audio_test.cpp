#include <iostream>
#include <vector>
#include <cstdint>
#include <cstdlib>
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

    // GET INPUT AND OUTPUT DEVICE INFO
    PaDeviceIndex inputDevice = Pa_GetDefaultInputDevice(); // get device number (input device as microphone)
    if (inputDevice == paNoDevice) {    // no mic
        std::cerr << "No input device (no mic)\n";
        Pa_Terminate();
        return 1;
    }
    std::cout << inputDevice << std::endl;

    const PaDeviceInfo* inputInfo = Pa_GetDeviceInfo(inputDevice);
    if (inputInfo == nullptr) {
        std::cerr << "Could not get input device info\n";
        Pa_Terminate();
        return 1;
    }
    // std::cout << "Device name: " << inputInfo->name << std::endl;
    // std::cout << "How many input chanel it support: " << inputInfo->maxInputChannels << std::endl;
    // std::cout << "normal sample rate: " << inputInfo->defaultSampleRate << std::endl;

    PaDeviceIndex outputDevice = Pa_GetDefaultOutputDevice(); // get device number (input device as microphone)
    if (outputDevice == paNoDevice) {    // no mic
        std::cerr << "No output device (no speakers) \n";
        Pa_Terminate();
        return 1;
    }
    std::cout << outputDevice << std::endl;

    const PaDeviceInfo* outputInfo = Pa_GetDeviceInfo(outputDevice);
    if (outputInfo == nullptr) {
        std::cerr << "Could not get output device info\n";
        Pa_Terminate();
        return 1;
    }
    // std::cout << "Device name: " << outputInfo->name << std::endl;
    // std::cout << "How many input chanel it support: " << outputInfo->maxOutputChannels << std::endl;
    // std::cout << "normal sample rate: " << outputInfo->defaultSampleRate << std::endl;

    // RECORDING MY VOICE INTO MEMORY
    PaStream * stream = nullptr;
    PaError errorOpenStream = Pa_OpenDefaultStream(&stream, CHANNELS, 0, paInt16, SAMPLE_RATE, FRAMES_PER_BUFFER, nullptr, nullptr);   // set up the pipe from mic to program
    if (!checkError(errorOpenStream)) {
        Pa_Terminate();
        return 1;
    }
    // start stream
    PaError startStreamErr = Pa_StartStream(stream);    // turn the mic on
    if (!checkError(startStreamErr)) {
        Pa_CloseStream(stream);
        Pa_Terminate();
        return 1;
    }

    // read stream (get my voice)
    std::vector<int16_t> record;
    int16_t chunk[FRAMES_PER_BUFFER];
    int numChunk = SAMPLE_RATE * 5 / FRAMES_PER_BUFFER; // how many loops to record
    std::cout << "Recording for 5 seconds... speak now\n";
    for (int i = 0; i < numChunk; i++) {
        PaError readStream = Pa_ReadStream(stream, chunk, FRAMES_PER_BUFFER);
        if (readStream == paInputOverflowed) {
            std::cout << "Overflow\n";
        }
        else if (readStream != paNoError) {
            std::cout << checkError(readStream);
            break;
        }
        record.insert(record.end(), chunk, chunk + FRAMES_PER_BUFFER);  // start = chunk, end = chun+frame, add everything from start to end
    }
    std::cout << "Recording size" << record.size() << std::endl;
    Pa_StopStream(stream);
    Pa_CloseStream(stream);

    int value = 0;
    for (int16_t sample : record) {
        int result = std::abs(static_cast<int>(sample));
        if (result > value) {
            value = result;
        }
    }
    std::cout << "largest sample: " << value << std::endl;

    // PLAY RECORDING BACK
    PaError errorRecordStream = Pa_OpenDefaultStream(&stream, 0, CHANNELS, paInt16, SAMPLE_RATE, FRAMES_PER_BUFFER, nullptr, nullptr);
    if (!checkError(errorRecordStream)) {
        Pa_Terminate();
        return 1;
    }
    PaError startRecordStreamErr = Pa_StartStream(stream);    // turn the speakers on
    if (!checkError(startRecordStreamErr)) {
        Pa_CloseStream(stream);
        Pa_Terminate();
        return 1;
    }
    std::cout << "Playing back\n";
    for (int i = 0; i < numChunk; i++) {
        PaError writeStream = Pa_WriteStream(stream, record.data() + i * FRAMES_PER_BUFFER, FRAMES_PER_BUFFER);    // where this chunk start inside record
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


    // LOOP BACK (MY VOICE SENDS TO SPEAKER IMMEDIATELY AFTER RECORDING)

    Pa_Terminate();
    return 0;
}