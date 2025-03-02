#ifndef INSTRUCTION_HEADER
#define INSTRUCTION_HEADER

#include "util.h"


#define AMOUNT_INSTRUCTIONS 16

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