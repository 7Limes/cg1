#include "stdint.h"

#ifndef AUDIO_DEFS_HEADER
#define AUDIO_DEFS_HEADER

#define AUDIO_SAMPLE_RATE 44100  // Hz

#define AMOUNT_AUDIO_CHANNELS 4
#define AMOUNT_WAVEFORMS 4


typedef enum {
    SQUARE,
    TRIANGLE,
    SAWTOOTH,
    NOISE
} Waveform;


typedef struct {
    Waveform waveform;
    uint16_t frequency;
    uint16_t volume;
    double phase;
} Channel;

#endif
