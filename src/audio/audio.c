#include <SDL2/SDL_audio.h>
#include "program.h"
#include "audio_defs.h"
#include "math.h"
#include "time.h"


int init_audio(ProgramContext *program_context, int32_t tickrate) {
    uint32_t buffer_size = AUDIO_SAMPLE_RATE / tickrate * 1.5;
    program_context->audio_buffer_size = buffer_size;

    SDL_AudioSpec spec;
    spec.freq = AUDIO_SAMPLE_RATE;
    spec.format = AUDIO_S16SYS;  // Signed 16-bit samples
    spec.channels = 1;
    spec.samples = buffer_size;
    spec.callback = NULL;

    // Open audio device
    SDL_AudioDeviceID device = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);
    if (device == 0) {
        return -1;
    }
    program_context->audio_device_id = device;

    // Initialize channels
    for (size_t i = 0; i < AMOUNT_AUDIO_CHANNELS; i++) {
        program_context->audio_channels[i] = (Channel) {
            .waveform = SQUARE,
            .frequency = 0,
            .volume = 0,
            .phase = 0.0
        };
    }

    // Create audio buffer
    program_context->audio_buffer = calloc(buffer_size, sizeof(int16_t));
    if (!program_context->audio_buffer) {
        return -2;
    }

    // Seed RNG for noise
    srand(time(NULL));

    // Start playing audio
    SDL_PauseAudioDevice(device, 0);

    return 0;
}


static inline int16_t generate_square(Channel *channel) {
    return channel->phase < 0.5 ? channel->volume : -channel->volume;
}


static inline int16_t generate_triangle(Channel *channel) {
    return (fabs(4 * channel->phase - 2) - 1) * channel->volume;
}


static inline int16_t generate_sawtooth(Channel *channel) {
    return (channel->phase - 0.5) * 2 * channel->volume;
}


static inline int16_t generate_noise(Channel *channel) {
    return rand() % channel->volume - (channel->volume / 2);
}


static inline int16_t generate_sample(Channel *channel) {
    int16_t sample;
    switch (channel->waveform) {
        case SQUARE:
        sample = generate_square(channel);
        break;
        case TRIANGLE:
            sample = generate_triangle(channel);
            break;
        case SAWTOOTH:
            sample = generate_sawtooth(channel);
            break;
        case NOISE:
            sample = generate_noise(channel);
            break;
    }

    // Increment phase
    channel->phase += channel->frequency / (double) AUDIO_SAMPLE_RATE;
    if (channel->phase >= 1.0) {
        channel->phase -= 1;
    }
    
    return sample;
}


void mix_channels(ProgramContext *program_context, uint32_t samples_to_generate) {
    // Clear audio buffer
    memset(program_context->audio_buffer, 0, samples_to_generate * sizeof(int16_t));

    // Generate and mix samples
    for (uint32_t i = 0; i < samples_to_generate; i++) {
        int32_t sample_sum = 0;
        for (uint32_t j = 0; j < AMOUNT_AUDIO_CHANNELS; j++) {
            Channel *channel = &program_context->audio_channels[j];
            if (channel->volume) {
                sample_sum += generate_sample(channel);
            }
        }
        int16_t mixed_sample = sample_sum / AMOUNT_AUDIO_CHANNELS;
        program_context->audio_buffer[i] = mixed_sample;
    }
}


int audio_tick(ProgramContext *program_context) {
    const uint32_t amount_queued_bytes = SDL_GetQueuedAudioSize(program_context->audio_device_id);
    const uint32_t samples_to_generate = program_context->audio_buffer_size - (amount_queued_bytes / sizeof(int16_t));
    printf("queued samples: %ld, samples to gen: %d\n", amount_queued_bytes / sizeof(int16_t), samples_to_generate);
    mix_channels(program_context, samples_to_generate);

    return SDL_QueueAudio(
        program_context->audio_device_id,
        program_context->audio_buffer, 
        samples_to_generate * sizeof(int16_t)
    );
}