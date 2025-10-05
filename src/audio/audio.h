#include <stdint.h>
#include "program.h"

#ifndef AUDIO_HEADER
#define AUDIO_HEADER


int init_audio(ProgramContext *program_context, int32_t tickrate);

int audio_tick(ProgramContext *program_context);

#endif
