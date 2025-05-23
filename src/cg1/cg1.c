/*
A g1 virtual machine implementation made with SDL2.

By Miles Burkart
https://github.com/7Limes
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "cJSON.h"
#include "program.h"
#include "instruction.h"
#include "util.h"
#include "cpu_primitives.h"


#define FLAG_BUFFER_SIZE 128

#define INSTRUCTION_ARGUMENT_BUFFER_SIZE 5


typedef int (*InstructionFunction)(ProgramContext *, int32_t *);


struct FlagData {
    bool show_fps, disable_log;
    uint32_t pixel_size;
};


void error(char *message) {
    printf("\x1b[31mRUNTIME ERROR: %s\n", message);
}

void out_of_bounds_error(int32_t address) {
    char err_buff[256];
    snprintf(err_buff, 256, "Tried to access out of bounds memory at address %d\n", address);
    error(err_buff);
}

void zero_division_error() {
    error("Division by zero.");
}


// Replaces `arguments` with either values in program memory or raw numbers then stores them in `parsed_arguments`.
int parse_arguments(int32_t *parsed_arguments, ProgramContext *program_context, Argument *arguments, byte argument_count) {
    for (size_t i = 0; i < argument_count; i++) {
        Argument arg = arguments[i];
        if (arg.type == 1) {  // Address
            #ifdef ENABLE_G1_RUNTIME_ERRORS
                if (arg.value < 0 || arg.value >= program_context->memory_size) {
                    out_of_bounds_error(arg.value);
                    return -1;
                }
            #endif
            parsed_arguments[i] = program_context->memory[arg.value];
        }
        else {  // Integer literal
            parsed_arguments[i] = arg.value;
        }
    }
    return 0;
}


int set_memory_value(int32_t dest, int32_t value, ProgramContext *program_context) {
    #ifdef ENABLE_G1_RUNTIME_ERRORS
        if (dest < 0 || dest >= program_context->memory_size) {
            out_of_bounds_error(dest);
            return -1;
        }
    #endif

    program_context->memory[dest] = value;
    return 0;
}


int ins_mov(ProgramContext *program_context, int32_t *args) {
    return set_memory_value(args[0], args[1], program_context);
}

int ins_movp(ProgramContext *program_context, int32_t *args) {
    #ifdef ENABLE_G1_RUNTIME_ERRORS
        if (args[1] < 0 || args[1] >= program_context->memory_size) {
            out_of_bounds_error(args[1]);
            return -2;
        }
    #endif

    return set_memory_value(args[0], program_context->memory[args[1]], program_context);
}

int ins_add(ProgramContext *program_context, int32_t *args) {
    return set_memory_value(args[0], args[1] + args[2], program_context);
}

int ins_sub(ProgramContext *program_context, int32_t *args) {
    return set_memory_value(args[0], args[1] - args[2], program_context);
}

int ins_mul(ProgramContext *program_context, int32_t *args) {
    return set_memory_value(args[0], args[1] * args[2], program_context);
}

int ins_div(ProgramContext *program_context, int32_t *args) {
    #ifdef ENABLE_G1_RUNTIME_ERRORS
        if (args[2] == 0) {
            zero_division_error();
            return -2;
        }
    #endif

    return set_memory_value(args[0], args[1] / args[2], program_context);
}

int ins_mod(ProgramContext *program_context, int32_t *args) {
    #ifdef ENABLE_G1_RUNTIME_ERRORS
        if (args[2] == 0) {
            zero_division_error();
            return -1;
        }
    #endif

    int32_t mod = args[1] % args[2];
    if (mod != 0 && (mod < 0) ^ (args[2] < 0)) {
        mod += args[2];
    }
    return set_memory_value(args[0], mod, program_context);
}

int ins_less(ProgramContext *program_context, int32_t *args) {
    return set_memory_value(args[0], args[1] < args[2], program_context);

}

int ins_equal(ProgramContext *program_context, int32_t *args) {
    return set_memory_value(args[0], args[1] == args[2], program_context);
}

int ins_not(ProgramContext *program_context, int32_t *args) {
    return set_memory_value(args[0], !args[1], program_context);
}

int ins_jmp(ProgramContext *program_context, int32_t *args) {
    if (args[1]) {
        program_context->program_counter = args[0]-1;
    }
    return 0;
}

int ins_color(ProgramContext *program_context, int32_t *args) {
    program_context->color = SDL_MapRGBA(program_context->render_surface->format, args[0], args[1], args[2], 255);
    return 0;
}

int ins_point(ProgramContext *program_context, int32_t *args) {
    surf_draw_point(program_context->render_surface, args[0], args[1], program_context->color);
    return 0;
}

int ins_line(ProgramContext *program_context, int32_t *args) {
    surf_draw_line(program_context->render_surface, args[0], args[1], args[2], args[3], program_context->color);
    return 0;
}

int ins_rect(ProgramContext *program_context, int32_t *args) {
    surf_draw_rect(program_context->render_surface, args[0], args[1], args[2], args[3], program_context->color);
    return 0;
}

int ins_log(ProgramContext *program_context, int32_t *args) {
    printf("%d\n", args[0]);
    return 0;
}

int ins_getp(ProgramContext *program_context, int32_t *args) {
    SDL_Surface *surf = program_context->render_surface;
    uint32_t *pixels = (uint32_t*) surf->pixels;
    uint32_t raw_pixel = pixels[args[1] + (args[2] * surf->w)];
    uint8_t r, g, b;
    SDL_GetRGB(raw_pixel, surf->format, &r, &g, &b);
    int32_t pixel_int = (int32_t) ((b << 16) | (g << 8) | r);
    set_memory_value(args[0], pixel_int, program_context);
}

InstructionFunction INSTRUCTION_FUNCTIONS[AMOUNT_INSTRUCTIONS] = {
    ins_mov, ins_movp,
    ins_add, ins_sub, ins_mul, ins_div, ins_mod,
    ins_less, ins_equal, ins_not,
    ins_jmp,
    ins_color, ins_point, ins_line, ins_rect,
    ins_log,
    ins_getp
};

#define LOG_OPCODE 15


int run_program_thread(const ProgramState *program_state, size_t index, struct FlagData *flag_data) {
    ProgramContext *program_context = program_state->context;
    ProgramData *program_data = program_state->data;
    
    program_context->program_counter = index;
    size_t instruction_count = program_data->instruction_count;
    while (program_context->program_counter < instruction_count) {
        Instruction instruction = program_data->instructions[program_context->program_counter];

        // Skip log instructions if they're disabled
        if (instruction.opcode == LOG_OPCODE && flag_data->disable_log) {
            program_context->program_counter++;
            continue;
        }

        // Parse instruction arguments
        uint32_t parsed_arguments[INSTRUCTION_ARGUMENT_BUFFER_SIZE];
        int parse_args_response = parse_arguments(parsed_arguments, program_context, instruction.arguments, ARGUMENT_COUNTS[instruction.opcode]);
        if (parse_args_response < 0) {
            return -1;
        }
        
        // Call the instruction function 
        int instruction_response = INSTRUCTION_FUNCTIONS[instruction.opcode](program_context, parsed_arguments);
        if (instruction_response != 0) {
            return -2;
        }

        program_context->program_counter++;
    }

    return 0;
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
    SDL_Window *win = SDL_CreateWindow("cg1", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);

    if (!win) {
        print_sdl_error("Failed to create window");
        return NULL;
    }

    return win;
}

void quit_sdl(SDL_Window *win, SDL_Renderer *renderer, SDL_Surface *render_surface) {
    if (win) {
        SDL_DestroyWindow(win);
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (render_surface) {
        SDL_FreeSurface(render_surface);
    }
    SDL_Quit();
}


void parse_flags(struct FlagData* flag_data, const char* flags) {
    flag_data->show_fps = false;
    flag_data->disable_log = false;
    flag_data->pixel_size = 1;

    if (flags[0] == '\0') {  // No flags provided
        return;
    }

    SplitString ss;
    ss_new(&ss, flags, ' ');

    char flag_buffer[FLAG_BUFFER_SIZE];
    while (true) {
        int ss_next_response = ss_next(flag_buffer, &ss, FLAG_BUFFER_SIZE);
        if (ss_next_response != 0) {
            break;
        }

        if (strcmp(flag_buffer, "--show_fps") == 0 || strcmp(flag_buffer, "-fps") == 0) {
            flag_data->show_fps = true;
        }
        else if (strcmp(flag_buffer, "--scale") == 0 || strcmp(flag_buffer, "-s") == 0) {
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
            flag_data->pixel_size = possible_pixel_size;
        }
        else if (strcmp(flag_buffer, "--disable_log") == 0 || strcmp(flag_buffer, "-dl") == 0) {
            flag_data->disable_log = true;
        }
        else {
            printf("Unrecognized flag \"%s\".\n", flag_buffer);
        }
    }
}


int init_program_state(const char *file_path, ProgramState *program_state) {
    int program_state_response;
    const char *extension = strrchr(file_path, '.') + 1;
    if (strcmp(extension, "g1b") == 0) {  // Binary format
        byte *program_bytes;
        size_t bytes_length;
        int file_read_response = read_file_bytes(&program_bytes, &bytes_length, file_path);
        if (file_read_response < 0) {
            return -1;
        }
        program_state_response = init_program_state_binary(program_state, program_bytes, bytes_length);
    }
    else {  // Assume JSON format
        cJSON *json = json_from_file(file_path);
        if (!json) {
            return -1;
        }
        program_state_response = init_program_state_json(program_state, json);
        cJSON_Delete(json);
    }

    if (program_state_response < 0) {
        return -2;
    }
    return 0;
}


int init_sdl(SDL_Window **win, SDL_Renderer **renderer, SDL_Surface **render_surface, uint32_t window_width, uint32_t window_height, uint32_t pixel_size) {
    *win = create_window(window_width, window_height);
    if (!win) {
        print_sdl_error("Failed to create SDL window");
        return -1;
    }

    // Create SDL renderer
    *renderer = SDL_CreateRenderer(*win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        print_sdl_error("Failed to create SDL renderer");
        quit_sdl(*win, NULL, NULL);
        return -2;
    }

    // Scale renderer
    int set_scale_response = SDL_RenderSetScale(*renderer, pixel_size, pixel_size);
    if (set_scale_response < 0) {
        print_sdl_error("Failed to set renderer scale");
        quit_sdl(*win, *renderer, NULL);
        return -3;
    }

    *render_surface = SDL_CreateRGBSurface(0, window_width, window_height, 32, 0, 0, 0, 0);
    if (!render_surface) {
        print_sdl_error("Failed to create render texture");
        quit_sdl(*win, *renderer, *render_surface);
        return -3;
    }

    return 0;
}


int program_tick_loop(ProgramState *program_state, const Uint8 *keyboard, struct FlagData *flag_data) {
    ProgramData *program_data = program_state->data;
    ProgramContext *program_context = program_state->context;

    Uint32 target_frame_time = 1000 / program_data->tickrate;
    Uint64 last_frame_time, start_frame_time;
    int32_t delta_ms = 0;

    SDL_Texture *present_texture;

    SDL_Rect dest_rect = {0, 0, flag_data->pixel_size * program_data->width, flag_data->pixel_size * program_data->height};
    
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
        
        update_reserved_memory(program_state, keyboard, delta_ms);
        int run_thread_response = run_program_thread(program_state, program_data->tick_index, flag_data);
        if (run_thread_response < 0) {
            return -1;
        }

        present_texture = SDL_CreateTextureFromSurface(program_context->renderer, program_context->render_surface);
        SDL_RenderCopy(program_context->renderer, present_texture, NULL, &dest_rect);
        SDL_DestroyTexture(present_texture);
        SDL_RenderPresent(program_context->renderer);

        uint64_t frame_time = SDL_GetTicks64() - start_frame_time;
        if (frame_time < target_frame_time) {
            SDL_Delay(target_frame_time - frame_time);
        }
    }
    
    return 0;
}


int run_file(const char *file_path, const char* flags) {
    // Parse flags
    struct FlagData flag_data;
    parse_flags(&flag_data, flags);

    // Create program state
    ProgramData program_data = {0};
    ProgramContext program_context = {0};
    ProgramState program_state = {&program_data, &program_context};
    int program_state_response = init_program_state(file_path, &program_state);
    if (program_state_response < 0) {
        return -1;
    }

    // Initialize SDL
    SDL_Window *win;
    SDL_Renderer *renderer;
    SDL_Surface *render_surface;
    uint32_t window_width = program_data.width*flag_data.pixel_size;
    uint32_t window_height = program_data.height*flag_data.pixel_size;
    int sdl_response = init_sdl(&win, &renderer, &render_surface, window_width, window_height, flag_data.pixel_size);
    if (sdl_response < 0) {
        free_program_state(&program_state);
        return -2;
    }
    program_context.renderer = renderer;
    program_context.render_surface = render_surface;
    program_context.color = 0;

    const Uint8 *keyboard = SDL_GetKeyboardState(NULL);
    
    // Jump to start label if it is there
    if (program_data.start_index != -1) {
        update_reserved_memory(&program_state, keyboard, 0);
        int run_thread_response = run_program_thread(&program_state, program_data.start_index, &flag_data);
        if (run_thread_response < 0) {
            quit_sdl(win, renderer, render_surface);
            free_program_state(&program_state);
            return -3;
        }
    }
    if (program_data.tick_index == -1) {
        quit_sdl(win, renderer, render_surface);
        free_program_state(&program_state);
        return 1;
    }

    int tick_loop_response = program_tick_loop(&program_state, keyboard, &flag_data);
    if (tick_loop_response < 0) {
        quit_sdl(win, renderer, render_surface);
        free_program_state(&program_state);
        return -4;
    }

    quit_sdl(win, renderer, render_surface);
    free_program_state(&program_state);
    return 0;
}