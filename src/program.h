#ifndef PROGRAM_HEADER
#define PROGRAM_HEADER

#include <stdlib.h>
#include <SDL2/SDL.h>
#include "cJSON.h"
#include "instruction.h"


// Stores static information about a program. (instructions, program metadata, etc.)
typedef struct {
    size_t instruction_count;
    Instruction *instructions;

    int32_t start_index, tick_index;
    int32_t memory_size, width, height, tickrate;

} ProgramData;

// Stores dynamic information about a program. (memory, program counter, etc.)
typedef struct {
    int32_t *memory;
    size_t program_counter;

    SDL_Renderer *renderer;

} ProgramContext;


typedef struct {
    ProgramData *data;
    ProgramContext *context;
} ProgramState;


// Initialize `program_state` from JSON format.
int init_program_state_json(ProgramState *program_state, ProgramData *program_data, ProgramContext *program_context, cJSON *program_data_json);

// Initialize `program_state` from binary format.
int init_program_state_binary(ProgramState *program_state, ProgramData *program_data, ProgramContext *program_context, byte *program_bytes, size_t bytes_length);

#endif