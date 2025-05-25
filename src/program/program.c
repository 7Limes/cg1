/*
    Contains ProgramData, ProgramContext, and ProgramState definitions,
    as well as functions for initializing the program state from a JSON or g1b file.
*/

#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
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
    size_t memory_size;  // Also store memory size here so we can do bounds checks

    Uint32 color;
    SDL_Window *win;
    SDL_Renderer *renderer;
    SDL_Surface *render_surface;
    TTF_Font *font;

} ProgramContext;


typedef struct {
    ProgramData *data;
    ProgramContext *context;
} ProgramState;


// Allocates memory for the program and records it in `program_context`
int init_program_context(ProgramContext *program_context, int32_t memory_size) {
    program_context->memory = calloc(memory_size, sizeof(int32_t));
    program_context->memory_size = memory_size;
    if (!program_context->memory) {
        printf("Failed to allocate program memory.\n");
        return -1;
    }
    return 0;
}


// Load all data entries from `data_array` into memory
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


int init_program_state_json(ProgramState *program_state, cJSON *program_data_json) {
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

    ProgramData *program_data = program_state->data;
    program_data->instruction_count = instruction_count;
    program_data->instructions = instructions;

    program_data->start_index = get_json_int(program_data_json, "start");
    program_data->tick_index = get_json_int(program_data_json, "tick");

    cJSON *meta = cJSON_GetObjectItem(program_data_json, "meta");
    program_data->memory_size = get_json_int(meta, "memory");
    program_data->width = get_json_int(meta, "width");
    program_data->height = get_json_int(meta, "height");
    program_data->tickrate = get_json_int(meta, "tickrate");

    int program_context_response = init_program_context(program_state->context, program_data->memory_size);
    if (program_context_response < 0) {
        return -4;
    }
    cJSON *data_array = cJSON_GetObjectItem(program_data_json, "data");  // Data entries are optional, so it's okay if this is `NULL`.
    add_data_entries_json(program_state->context, data_array);

    return 0;
}


void add_data_entries_binary(ProgramContext* program_context, uint32_t data_entry_count, BytesIterator *iter) {
    for (uint32_t i = 0; i < data_entry_count; i++) {
        uint32_t entry_address, entry_size;
        bi_next_n(&entry_address, iter, 4);
        bi_next_n(&entry_size, iter, 4);
        for (uint32_t j = 0; j < entry_size; j++) {
            int32_t value;
            bi_next_n(&value, iter, 4);
            program_context->memory[entry_address+j] = value;
        }
    }
}


int init_program_state_binary(ProgramState *program_state, byte *program_bytes, size_t bytes_length) {
    // Create the iterator
    BytesIterator iter;
    bi_new(&iter, program_bytes, bytes_length);

    // Initialize buffers for fields whose sizes don't match the binary format size
    uint16_t u16_buffer = 0;
    uint32_t u32_buffer = 0;

    // Check for "g1" signature
    uint16_t signature;
    bi_next_n(&signature, &iter, 2);
    if (signature != 0x6731) {
        return -1;
    }

    // Get program metadata
    ProgramData *program_data = program_state->data;
    bi_next_n(&program_data->memory_size, &iter, 4);
    bi_next_n(&u16_buffer, &iter, 2);
    program_data->width = (uint32_t) u16_buffer;
    bi_next_n(&u16_buffer, &iter, 2);
    program_data->height = (uint32_t) u16_buffer;
    bi_next_n(&u16_buffer, &iter, 2);
    program_data->tickrate = (uint32_t) u16_buffer;
    bi_next_n(&program_data->tick_index, &iter, 4);
    bi_next_n(&program_data->start_index, &iter, 4);

    // Create instructions
    bi_next_n(&u32_buffer, &iter, 4);
    program_data->instruction_count = (size_t) u32_buffer;
    Instruction* instructions = parse_instructions_binary(program_data->instruction_count, &iter);
    if (!instructions) {
        return -2;
    }
    program_data->instructions = instructions;

    // Set data entries
    uint32_t data_entry_count;
    bi_next_n(&data_entry_count, &iter, 4);
    int program_context_response = init_program_context(program_state->context, program_data->memory_size);
    if (program_context_response < 0) {
        return -3;
    }
    add_data_entries_binary(program_state->context, data_entry_count, &iter);

    return 0;
}