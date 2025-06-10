/*
    Contains ProgramData, ProgramContext, and ProgramState definitions,
    as well as functions for initializing the program state from a JSON or g1b file.
*/

#ifndef PROGRAM_HEADER
#define PROGRAM_HEADER

#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
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
    size_t program_counter;
    size_t memory_size;  // Also store memory size here so we can do bounds checks
    int32_t *memory;

    SDL_Window *win;
    SDL_Renderer *renderer;
    SDL_Surface *render_surface;
    TTF_Font *font;
    Uint32 color;

} ProgramContext;


typedef struct {
    ProgramData *data;
    ProgramContext *context;
} ProgramState;


// Initialize `program_state` from JSON format.
int init_program_state_json(ProgramState *program_state, cJSON *program_data_json);

// Initialize `program_state` from binary format.
int init_program_state_binary(ProgramState *program_state, byte *program_bytes, size_t bytes_length);

#endif