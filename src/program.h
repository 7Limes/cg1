#ifndef PROGRAM_HEADER
#define PROGRAM_HEADER

#include <stdlib.h>
#include <SDL2/SDL.h>
#include "cJSON.h"
#include "instruction.h"

typedef struct {
    size_t instruction_count;
    Instruction *instructions;

    int32_t start_index, tick_index;
    int32_t memory_size, width, height, tickrate;

} ProgramData;


typedef struct {
    int32_t *memory;
    size_t program_counter;

    SDL_Renderer *renderer;

} ProgramContext;


typedef struct {
    ProgramData *data;
    ProgramContext *context;
} ProgramState;


void add_data_entries_json(ProgramContext* program_context, cJSON* data_array);

int init_program_context_json(ProgramContext *program_context, ProgramData *program_data, cJSON *data_array);

int init_program_state_json(ProgramState *program_state, ProgramData *program_data, ProgramContext *program_context, cJSON *program_data_json);

#endif