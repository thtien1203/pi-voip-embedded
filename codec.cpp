#include "codec.hpp"
#include "audio.hpp"
#include <opus/opus.h>
#include <iostream>

static OpusEncoder * encoder = nullptr;
static OpusDecoder * decoder = nullptr;
// Sending: mic → readChunk → Opus encode (1920 bytes → ~60 bytes) → sendPacket
// Receiving: packet → Opus decode (~60 bytes → 1920 bytes) → jitterPush → playback
bool codecInit() {
    int err = 0;
    // create encoder
    encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS,OPUS_APPLICATION_VOIP, &err);
    if (err != OPUS_OK) {
        std::cerr << "Encoder error: " << opus_strerror(err) << "\n";
        return false;
    } 
    
    // create decoder
    decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &err);
    if (err != OPUS_OK) {
        opus_encoder_destroy(encoder);
        encoder = nullptr;
        std::cout << "Decoder error: " << opus_strerror(err) << "\n";
        return false;
    } 
    return true;
}
int encodeChunk(const int16_t* pcm_in, uint8_t* out, int maxBytes) {
    return opus_encode(encoder, pcm_in, FRAMES_PER_BUFFER, out, maxBytes);
}

bool decodeChunk(const uint8_t* data, int size, int16_t* pcm_out) {
    int samples = opus_decode(decoder, data, size, pcm_out, FRAMES_PER_BUFFER, 0);
    return samples == FRAMES_PER_BUFFER;
}

void codecDestroy() {
    if (encoder != nullptr) {
        opus_encoder_destroy(encoder);
        encoder = nullptr;
    }
    if (decoder != nullptr) {
        opus_decoder_destroy(decoder);
        decoder = nullptr;
    }

}