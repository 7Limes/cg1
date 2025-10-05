/*
    Functions for parsing JSON and g1b files into a list of instructions.
*/

#ifndef INSTRUCTION_HEADER
#define INSTRUCTION_HEADER

#include "util.h"

#define AMOUNT_INSTRUCTIONS 18

#define OP_MOV 0
#define OP_MOVP 1
#define OP_ADD 2
#define OP_SUB 3
#define OP_MUL 4
#define OP_DIV 5
#define OP_MOD 6
#define OP_LESS 7
#define OP_EQUAL 8
#define OP_NOT 9
#define OP_JMP 10
#define OP_COLOR 11
#define OP_POINT 12
#define OP_LINE 13
#define OP_RECT 14
#define OP_LOG 15
#define OP_GETP 16
#define OP_SETCH 17

extern const byte ARGUMENT_COUNTS[];

/*
A g1 instruction argument.
type:
- `0` - Integer literal
- `1` - Address
*/
typedef struct {
    byte type;
    int32_t value;
} Argument;

// A single g1 instruction.
typedef struct {
    byte opcode;
    Argument arguments[4];
} Instruction;

// Get a uint32 from `json`.
int32_t get_json_int(cJSON *json, const char *name);

// Convert a json instructions array into an array of `Instruction` structs.
Instruction* parse_instructions_json(cJSON *instructions_json);

// Create an array of `Instruction` structs from an iterator starting at the instruction array index.
Instruction* parse_instructions_binary(size_t instruction_count, BytesIterator *iter);

#endif