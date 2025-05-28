/*
    Instruction implementations
*/

#ifndef INSTRUCTION_IMPL_HEADER
#define INSTRUCTION_IMPL_HEADER

#include <stdint.h>
#include "program.h"

#ifndef ENABLE_G1_GPU_RENDERING
    #include "cpu_primitives.h"
#endif


#define INSTRUCTION_ARGUMENT_BUFFER_SIZE 5


static inline void _error(char *message) {
    printf("\x1b[31mRUNTIME ERROR: %s\n", message);
}


static inline void _zero_division_error() {
    _error("Division by zero.");
}


static inline void _out_of_bounds_error(int32_t address) {
    char err_buff[256];
    snprintf(err_buff, 256, "Tried to access out of bounds memory at address %d\n", address);
    _error(err_buff);
}


// Instruction function definitions
static inline int _set_memory_value(int32_t dest, int32_t value, ProgramContext *program_context) {
    #ifdef ENABLE_G1_RUNTIME_ERRORS
        if (dest < 0 || dest >= program_context->memory_size) {
            _out_of_bounds_error(dest);
            return -1;
        }
    #endif

    program_context->memory[dest] = value;
    return 0;
}


static inline int _ins_mov(ProgramContext *program_context, int32_t *args) {
    return _set_memory_value(args[0], args[1], program_context);
}

static inline int _ins_movp(ProgramContext *program_context, int32_t *args) {
    #ifdef ENABLE_G1_RUNTIME_ERRORS
        if (args[1] < 0 || args[1] >= program_context->memory_size) {
            _out_of_bounds_error(args[1]);
            return -2;
        }
    #endif

    return _set_memory_value(args[0], program_context->memory[args[1]], program_context);
}

static inline int _ins_add(ProgramContext *program_context, int32_t *args) {
    return _set_memory_value(args[0], args[1] + args[2], program_context);
}

static inline int _ins_sub(ProgramContext *program_context, int32_t *args) {
    return _set_memory_value(args[0], args[1] - args[2], program_context);
}

static inline int _ins_mul(ProgramContext *program_context, int32_t *args) {
    return _set_memory_value(args[0], args[1] * args[2], program_context);
}

static inline int _ins_div(ProgramContext *program_context, int32_t *args) {
    #ifdef ENABLE_G1_RUNTIME_ERRORS
        if (args[2] == 0) {
            _zero_division_error();
            return -2;
        }
    #endif

    return _set_memory_value(args[0], args[1] / args[2], program_context);
}

static inline int _ins_mod(ProgramContext *program_context, int32_t *args) {
    #ifdef ENABLE_G1_RUNTIME_ERRORS
        if (args[2] == 0) {
            _zero_division_error();
            return -1;
        }
    #endif

    int32_t mod = args[1] % args[2];
    if (mod != 0 && (mod < 0) ^ (args[2] < 0)) {
        mod += args[2];
    }
    return _set_memory_value(args[0], mod, program_context);
}

static inline int _ins_less(ProgramContext *program_context, int32_t *args) {
    return _set_memory_value(args[0], args[1] < args[2], program_context);

}

static inline int _ins_equal(ProgramContext *program_context, int32_t *args) {
    return _set_memory_value(args[0], args[1] == args[2], program_context);
}

static inline int _ins_not(ProgramContext *program_context, int32_t *args) {
    return _set_memory_value(args[0], !args[1], program_context);
}

static inline int _ins_jmp(ProgramContext *program_context, int32_t *args) {
    if (args[1]) {
        program_context->program_counter = args[0]-1;
    }
    return 0;
}

static inline int _ins_color(ProgramContext *program_context, int32_t *args) {
    #ifdef ENABLE_G1_GPU_RENDERING
        SDL_SetRenderDrawColor(program_context->renderer, args[0], args[1], args[2], 255);
    #else
        program_context->color = SDL_MapRGBA(program_context->render_surface->format, args[0], args[1], args[2], 255);
    #endif
    return 0;
}

static inline int _ins_point(ProgramContext *program_context, int32_t *args) {
    #ifdef ENABLE_G1_GPU_RENDERING
        return SDL_RenderDrawPoint(program_context->renderer, args[0], args[1]);
    #else
        surf_draw_point(program_context->render_surface, args[0], args[1], program_context->color);
    #endif
    return 0;
}

static inline int _ins_line(ProgramContext *program_context, int32_t *args) {
    #ifdef ENABLE_G1_GPU_RENDERING
        return SDL_RenderDrawLine(program_context->renderer, args[0], args[1], args[2], args[3]);
    #else
        surf_draw_line(program_context->render_surface, args[0], args[1], args[2], args[3], program_context->color);
    #endif
    return 0;
}

static inline int _ins_rect(ProgramContext *program_context, int32_t *args) {
    #ifdef ENABLE_G1_GPU_RENDERING
        return SDL_RenderFillRect(program_context->renderer, &(SDL_Rect) {args[0], args[1], args[2], args[3]});
    #else
        surf_draw_rect(program_context->render_surface, args[0], args[1], args[2], args[3], program_context->color);
    #endif
    return 0;
}

static inline int _ins_log(ProgramContext *program_context, int32_t *args) {
    printf("%d\n", args[0]);
    return 0;
}

static inline int _ins_getp(ProgramContext *program_context, int32_t *args) {
    SDL_Surface *surf = program_context->render_surface;
    uint32_t *pixels = (uint32_t*) surf->pixels;
    uint32_t raw_pixel = pixels[args[1] + (args[2] * surf->w)];
    uint8_t r, g, b;
    SDL_GetRGB(raw_pixel, surf->format, &r, &g, &b);
    int32_t pixel_int = (int32_t) ((b << 16) | (g << 8) | r);
    _set_memory_value(args[0], pixel_int, program_context);
}


// Replaces `arguments` with either values in program memory or raw numbers then stores them in `parsed_arguments`.
static inline int _parse_arguments(int32_t *parsed_arguments, ProgramContext *program_context, const Argument *arguments, byte argument_count) {
    for (size_t i = 0; i < argument_count; i++) {
        Argument arg = arguments[i];
        if (arg.type == 1) {  // Address
            #ifdef ENABLE_G1_RUNTIME_ERRORS
                if (arg.value < 0 || arg.value >= program_context->memory_size) {
                    _out_of_bounds_error(arg.value);
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


// Runs a single instruction.
static inline int run_instruction(ProgramContext *program_context, const Instruction *ins) {
    // Parse instruction arguments
    byte argument_count = ARGUMENT_COUNTS[ins->opcode];
    int32_t args[INSTRUCTION_ARGUMENT_BUFFER_SIZE];
    int parse_args_response = _parse_arguments(args, program_context, ins->arguments, argument_count);
    if (parse_args_response < 0) {
        return -1;
    }
    
    // Run the corresponding instruction
    switch (ins->opcode) {
        case OP_MOV:
            return _ins_mov(program_context, args);
        case OP_MOVP:
            return _ins_movp(program_context, args);
        case OP_ADD:
            return _ins_add(program_context, args);
        case OP_SUB:
            return _ins_sub(program_context, args);
        case OP_MUL:
            return _ins_mul(program_context, args);
        case OP_DIV:
            return _ins_div(program_context, args);
        case OP_MOD:
            return _ins_mod(program_context, args);
        case OP_LESS:
            return _ins_less(program_context, args);
        case OP_EQUAL:
            return _ins_equal(program_context, args);
        case OP_NOT:
            return _ins_not(program_context, args);
        case OP_JMP:
            return _ins_jmp(program_context, args);
        case OP_COLOR:
            return _ins_color(program_context, args);
        case OP_POINT:
            return _ins_point(program_context, args);
        case OP_LINE:
            return _ins_line(program_context, args);
        case OP_RECT:
            return _ins_rect(program_context, args);
        case OP_LOG:
            #ifdef ENABLE_G1_LOG_INSTRUCTION
                return _ins_log(program_context, args);
            #else
                return 0;
            #endif
        case OP_GETP:
            return _ins_getp(program_context, args);
    }
}

#endif