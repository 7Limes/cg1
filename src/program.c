#include <stdlib.h>
#include <SDL2/SDL.h>
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


void add_data_entries_json(ProgramContext* program_context, cJSON* data_array) {
    if (data_array == NULL) {
        return;
    }
    size_t amount_entries = cJSON_GetArraySize(data_array);
    for (size_t i = 0; i < amount_entries; i++) {
        cJSON *entry = cJSON_GetArrayItem(data_array, i);
        size_t entry_address = cJSON_GetNumberValue(cJSON_GetArrayItem(entry, 0));
        cJSON *entry_data = cJSON_GetArrayItem(entry, 1);
        size_t entry_data_size = cJSON_GetArraySize(entry_data);
        for (size_t j = 0; j < entry_data_size; j++) {
            cJSON *memory_item = cJSON_GetArrayItem(entry_data, j);
            size_t item_address = entry_address + j;
            program_context->memory[item_address] = cJSON_GetNumberValue(memory_item);
        }
    }
}


int init_program_context_json(ProgramContext *program_context, ProgramData *program_data, cJSON *data_array) {
    program_context->memory = calloc(program_data->memory_size, sizeof(int32_t));
    if (!program_context->memory) {
        printf("Failed to allocate program memory.\n");
        return -1;
    }
    add_data_entries_json(program_context, data_array);
    return 0;
}


int init_program_state_json(ProgramState *program_state, ProgramData *program_data, ProgramContext *program_context, cJSON *program_data_json) {
    cJSON *instructions_json = cJSON_GetObjectItem(program_data_json, "instructions");
    if (!instructions_json) {
        printf("Could not find instructions array in JSON.\n");
        return -1;
    }
    if (!cJSON_IsArray(instructions_json)) {
        printf("JSON instructions object is not an array.\n");
        return -2;
    }
    size_t instruction_count = cJSON_GetArraySize(instructions_json);
    Instruction *instructions = parse_instructions_json(instructions_json);
    if (!instructions) {
        return -3;
    }

    program_data->instruction_count = instruction_count;
    program_data->instructions = instructions;

    program_data->start_index = get_json_int(program_data_json, "start");
    program_data->tick_index = get_json_int(program_data_json, "tick");

    cJSON *meta = cJSON_GetObjectItem(program_data_json, "meta");
    program_data->memory_size = get_json_int(meta, "memory");
    program_data->width = get_json_int(meta, "width");
    program_data->height = get_json_int(meta, "height");
    program_data->tickrate = get_json_int(meta, "tickrate");

    cJSON *data_array = cJSON_GetObjectItem(program_data_json, "data");  // Data entries are optional, so it's okay if this is `NULL`.
    int program_context_response = init_program_context_json(program_context, program_data, data_array);
    if (program_context_response < 0) {
        return -4;
    }

    program_state->data = program_data;
    program_state->context = program_context;
    return 0;
}