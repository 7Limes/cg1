/*
A g1 virtual machine implementation made with SDL2.

By Miles Burkart
11-27-2024
https://github.com/7Limes
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "cJSON.h"
#include "util.h"


#define FLAG_BUFFER_SIZE 64

#define INSTRUCTION_ARGUMENT_BUFFER_SIZE 5
#define AMOUNT_INSTRUCTIONS 15
const char *INSTRUCTIONS[AMOUNT_INSTRUCTIONS] = {
    "mov",
    "movp",
    "add",
    "sub",
    "mul",
    "div",
    "mod",
    "less",
    "equal",
    "not",
    "jmp",
    "color",
    "point",
    "line",
    "rect",
};

const byte ARGUMENT_COUNTS[AMOUNT_INSTRUCTIONS] = {2, 2, 3, 3, 3, 3, 3, 3, 3, 2, 2, 3, 2, 4, 4};


/*
    type:
        0=int
        1=address
*/
struct Argument {
    byte type;
    int32_t value;
};

struct Instruction {
    byte opcode;
    struct Argument *arguments;
};


typedef struct {
    size_t instruction_count;
    struct Instruction *instructions;

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


typedef void (*InstructionFunction)(ProgramContext *, int32_t *);


int32_t get_json_int(cJSON *json, const char *name) {
    cJSON *item = cJSON_GetObjectItem(json, name);
    if (item == NULL) {
        return -1;
    }
    return (int32_t) cJSON_GetNumberValue(item);
}


int parse_json_arguments(cJSON *arguments_json, struct Argument *instruction_args, byte argument_count) {
    for (size_t i = 0; i < argument_count; i++) {
        cJSON *argument_value = cJSON_GetArrayItem(arguments_json, i);

        if (cJSON_IsNumber(argument_value)) {
            struct Argument arg = {0, (int32_t) cJSON_GetNumberValue(argument_value)};
            instruction_args[i] = arg;
        }
        else if (cJSON_IsString(argument_value)) {
            char *address_string = cJSON_GetStringValue(argument_value);
            struct Argument arg = {1, (int32_t) atoi(address_string+1)};
            instruction_args[i] = arg;
        }
        else {
            char *argument_value_string = cJSON_Print(argument_value);
            printf("Got unexpected type at index %ld when parsing instruction arguments: \"%s\"\n", i, argument_value_string);
            return -1;
        }
    }
    return 0;
}


byte get_opcode(char *instruction_name) {
    for (size_t i = 0; i < AMOUNT_INSTRUCTIONS; i++) {
        if (strcmp(INSTRUCTIONS[i], instruction_name) == 0) {
            return i;
        }
    }
    return 255;
}


struct Instruction* parse_instructions(cJSON *instructions_json) {
    size_t instruction_count = cJSON_GetArraySize(instructions_json);
    struct Instruction *instructions = malloc(sizeof(struct Instruction) * instruction_count);
    if (!instructions) {
        printf("Failed to allocate memory for instructions.\n");
        return NULL;
    }

    for (size_t i = 0; i < instruction_count; i++) {
        cJSON *instruction_data = cJSON_GetArrayItem(instructions_json, i);
        char *instruction_name = cJSON_GetStringValue(cJSON_GetArrayItem(instruction_data, 0));
        byte opcode = get_opcode(instruction_name);
        if (opcode == 255) {
            free(instructions);
            printf("Unrecognized instruction at index %ld: \"%s\"\n", i, instruction_name);
            return NULL;
        }
        byte argument_count = ARGUMENT_COUNTS[opcode];
        struct Argument *instruction_args = malloc(sizeof(struct Argument)*argument_count);
        if (!instruction_args) {
            printf("Failed to allocate memory for instruction arguments.\n");
            return NULL;
        }
        int parse_arguments_response = parse_json_arguments(cJSON_GetArrayItem(instruction_data, 1), instruction_args, argument_count);
        if (parse_arguments_response < 0) {
            free(instructions);
            return NULL;
        }

        instructions[i] = (struct Instruction) {opcode, instruction_args};
    }
    return instructions;
}


void add_initial_data(ProgramContext* program_context, cJSON* data_array) {
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


int initialize_program_context(ProgramContext *program_context, ProgramData *program_data, cJSON *data_array) {
    program_context->memory = calloc(program_data->memory_size, sizeof(int32_t));
    if (!program_context->memory) {
        printf("Failed to allocate program memory.\n");
        return -1;
    }
    add_initial_data(program_context, data_array);
    return 0;
}


int create_program_state(ProgramState *program_state, ProgramData *program_data, ProgramContext *program_context, cJSON *program_data_json) {
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
    struct Instruction *instructions = parse_instructions(instructions_json);
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

    cJSON *data_array = cJSON_GetObjectItem(program_data_json, "data");  // Data is optional, so it's okay if this is `NULL`.
    int program_context_response = initialize_program_context(program_context, program_data, data_array);
    if (program_context < 0) {
        return -4;
    }

    program_state->data = program_data;
    program_state->context = program_context;
    return 0;
}


// Replaces `arguments` with either values in program memory or raw numbers then stores them in `parsed_arguments`.
void parse_arguments(int32_t *parsed_arguments, ProgramContext *program_context, struct Argument *arguments, byte argument_count) {
    for (size_t i = 0; i < argument_count; i++) {
        struct Argument arg = arguments[i];
        if (arg.type) {
            parsed_arguments[i] = program_context->memory[arg.value];
        }
        else {
            parsed_arguments[i] = arg.value;
        }
    }
}


void ins_mov(ProgramContext *program_context, int32_t *args) {
    program_context->memory[args[0]] = args[1];
}

void ins_movp(ProgramContext *program_context, int32_t *args) {
    program_context->memory[args[0]] = program_context->memory[args[1]];
}

void ins_add(ProgramContext *program_context, int32_t *args) {
    program_context->memory[args[0]] = args[1] + args[2];
}

void ins_sub(ProgramContext *program_context, int32_t *args) {
    program_context->memory[args[0]] = args[1] - args[2];
}

void ins_mul(ProgramContext *program_context, int32_t *args) {
    program_context->memory[args[0]] = args[1] * args[2];
}

void ins_div(ProgramContext *program_context, int32_t *args) {
    program_context->memory[args[0]] = args[1] / args[2];
}

void ins_mod(ProgramContext *program_context, int32_t *args) {
    program_context->memory[args[0]] = args[1] % args[2];
}

void ins_less(ProgramContext *program_context, int32_t *args) {
    program_context->memory[args[0]] = args[1] < args[2];
}

void ins_equal(ProgramContext *program_context, int32_t *args) {
    program_context->memory[args[0]] = args[1] == args[2];
}

void ins_not(ProgramContext *program_context, int32_t *args) {
    program_context->memory[args[0]] = !args[1];
}

void ins_jmp(ProgramContext *program_context, int32_t *args) {
    if (args[1]) {
        program_context->program_counter = args[0]-1;
    }
}

void ins_color(ProgramContext *program_context, int32_t *args) {
    SDL_SetRenderDrawColor(program_context->renderer, args[0], args[1], args[2], 255);
}

void ins_point(ProgramContext *program_context, int32_t *args) {
    SDL_RenderDrawPoint(program_context->renderer, args[0], args[1]);
}

void ins_line(ProgramContext *program_context, int32_t *args) {
    SDL_RenderDrawLine(program_context->renderer, args[0], args[1], args[2], args[3]);
}

void ins_rect(ProgramContext *program_context, int32_t *args) {
    SDL_RenderFillRect(program_context->renderer, &(SDL_Rect) {args[0], args[1], args[2], args[3]});
}

InstructionFunction INSTRUCTION_FUNCTIONS[AMOUNT_INSTRUCTIONS] = {
    ins_mov, ins_movp,
    ins_add, ins_sub, ins_mul, ins_div, ins_mod,
    ins_less, ins_equal, ins_not,
    ins_jmp,
    ins_color, ins_point, ins_line, ins_rect
};


void run_program_thread(const ProgramState *program_state, size_t index) {
    ProgramContext *program_context = program_state->context;
    ProgramData *program_data = program_state->data;

    program_context->program_counter = index;
    size_t instruction_count = program_data->instruction_count;
    while (program_context->program_counter < instruction_count) {
        struct Instruction instruction = program_data->instructions[program_context->program_counter];
        uint32_t parsed_arguments[INSTRUCTION_ARGUMENT_BUFFER_SIZE];
        parse_arguments(parsed_arguments, program_context, instruction.arguments, ARGUMENT_COUNTS[instruction.opcode]);
        INSTRUCTION_FUNCTIONS[instruction.opcode](program_context, parsed_arguments);
        program_context->program_counter++;
    }
}


void update_reserved_memory(const ProgramState *program_state, const Uint8 *keys, Uint64 delta_ms) {
    ProgramData *program_data = program_state->data;
    int32_t values[] = {
        keys[SDL_SCANCODE_RETURN],
        keys[SDL_SCANCODE_RSHIFT],
        keys[SDL_SCANCODE_Z],
        keys[SDL_SCANCODE_X],
        keys[SDL_SCANCODE_UP],
        keys[SDL_SCANCODE_DOWN],
        keys[SDL_SCANCODE_LEFT],
        keys[SDL_SCANCODE_RIGHT],
        program_data->memory_size,
        program_data->width,
        program_data->height,
        program_data->tickrate,
        delta_ms
    };
    memcpy(program_state->context->memory, values, sizeof(values));
}


void free_program_state(const ProgramState *program_state) {
    ProgramData *program_data = program_state->data;
    ProgramContext *program_context = program_state->context;
    for (size_t i = 0; i < program_data->instruction_count; i++) {
        free(program_data->instructions[i].arguments);
    }
    free(program_data->instructions);

    free(program_context->memory);
}


void print_sdl_error(const char *message) {
    printf("%s: \"%s\"\n", message, SDL_GetError());
}


SDL_Window* create_window(uint32_t width, uint32_t height) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        print_sdl_error("Failed to initialize SDL");
        return NULL;
    }
    SDL_Window *win = SDL_CreateWindow("cg1", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);

    if (!win) {
        print_sdl_error("Failed to create window");
        return NULL;
    }

    return win;
}

void quit_sdl(SDL_Window *win, SDL_Renderer *renderer) {
    SDL_DestroyWindow(win);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
}


struct FlagData {
    bool show_fps;
    uint32_t pixel_size;
};

void parse_flags(struct FlagData* flag_data, const char* flags) {
    bool show_fps = false;
    uint32_t pixel_size = 1;

    SplitString ss;
    ss_new(&ss, flags, ' ');

    char flag_buffer[FLAG_BUFFER_SIZE];
    while (true) {
        int ss_next_response = ss_next(flag_buffer, &ss, FLAG_BUFFER_SIZE);
        if (ss_next_response != 0) {
            break;
        }

        if (strcmp(flag_buffer, "-f") == 0) {
            show_fps = true;
        }
        else if (strcmp(flag_buffer, "-s") == 0) {
            ss_next_response = ss_next(flag_buffer, &ss, FLAG_BUFFER_SIZE);  // `flag_buffer` should now contain pixel size as a string
            if (ss_next_response != 0) {
                printf("Expected a value for pixel size flag.\n");
                continue;
            }
            uint32_t possible_pixel_size = atoi(flag_buffer);
            if (possible_pixel_size == 0) {
                printf("Expected numeric or nonzero value for pixel size flag.\n");
                continue;
            }
            pixel_size = possible_pixel_size;
        }
        else {
            printf("Unrecognized flag \"%s\".\n", flag_buffer);
        }
    }

    flag_data->show_fps = show_fps;
    flag_data->pixel_size = pixel_size;
}


int run_file(const char *file_path, const char* flags) {
    // Read JSON file
    cJSON *json = json_from_file(file_path);
    if (!json) {
        return -1;
    }

    // Create program state
    ProgramState program_state;
    ProgramData program_data;
    ProgramContext program_context;
    int program_state_response = create_program_state(&program_state, &program_data, &program_context, json);
    cJSON_Delete(json);
    if (program_state_response < 0) {
        return -2;
    }

    // Parse flags
    struct FlagData flag_data;
    parse_flags(&flag_data, flags);

    // Create SDL window
    uint32_t window_width = program_data.width*flag_data.pixel_size;
    uint32_t window_height = program_data.height*flag_data.pixel_size;
    SDL_Window *win = create_window(window_width, window_height);
    if (!win) {
        free_program_state(&program_state);
        return -3;
    }

    // Create SDL renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_DestroyWindow(win);
        SDL_Quit();
        free_program_state(&program_state);
        return -4;
    }

    // Scale renderer
    int set_scale_response = SDL_RenderSetScale(renderer, flag_data.pixel_size, flag_data.pixel_size);
    if (set_scale_response < 0) {
        print_sdl_error("Failed to set renderer scale");
        quit_sdl(win, renderer);
        free_program_state(&program_state);
        return -5;
    }
    program_context.renderer = renderer;

    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    // Jump to start label if it is there
    if (program_data.start_index != -1) {
        update_reserved_memory(&program_state, keys, 0);
        run_program_thread(&program_state, program_data.start_index);
    }

    Uint32 target_frame_time = 1000 / program_data.tickrate;
    Uint64 last_frame_time, start_frame_time;
    int32_t delta_ms = 0;

    bool running = true; 
    while (running) {
        start_frame_time = SDL_GetTicks64();
        delta_ms = start_frame_time - last_frame_time;
        last_frame_time = start_frame_time;

        SDL_Event e;
        while (SDL_PollEvent(&e) > 0) {
            switch (e.type) {
                case SDL_QUIT:
                    running = false;
                    break;
            }
        }
        SDL_PumpEvents();
        update_reserved_memory(&program_state, keys, delta_ms);
        run_program_thread(&program_state, program_data.tick_index);

        SDL_RenderPresent(renderer);
        uint64_t frame_time = SDL_GetTicks64() - start_frame_time;
        if (frame_time < target_frame_time) {
            SDL_Delay(target_frame_time - frame_time);
        }
    }

    quit_sdl(win, renderer);

    free_program_state(&program_state);
}