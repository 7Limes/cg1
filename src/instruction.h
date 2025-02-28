#include "util.h"


#define AMOUNT_INSTRUCTIONS 16

extern const byte ARGUMENT_COUNTS[];

/*
    type:
        0=int
        1=address
*/
typedef struct {
    byte type;
    int32_t value;
} Argument;

typedef struct {
    byte opcode;
    Argument arguments[4];
} Instruction;

int32_t get_json_int(cJSON *json, const char *name);

int parse_json_arguments(cJSON *arguments_json, Argument *instruction_args, byte argument_count);

byte get_opcode(char *instruction_name);

Instruction* parse_instructions_json(cJSON *instructions_json);