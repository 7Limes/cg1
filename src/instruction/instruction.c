#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "cJSON.h"
#include "util.h"


#define AMOUNT_INSTRUCTIONS 16
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
    "log"
};

const byte ARGUMENT_COUNTS[AMOUNT_INSTRUCTIONS] = {2, 2, 3, 3, 3, 3, 3, 3, 3, 2, 2, 3, 2, 4, 4, 1};


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


int32_t get_json_int(cJSON *json, const char *name) {
    cJSON *item = cJSON_GetObjectItem(json, name);
    if (item == NULL) {
        return -1;
    }
    return (int32_t) cJSON_GetNumberValue(item);
}


int parse_json_arguments(cJSON *arguments_json, Argument *instruction_args, byte argument_count) {
    for (size_t i = 0; i < argument_count; i++) {
        cJSON *argument_value = cJSON_GetArrayItem(arguments_json, i);

        if (cJSON_IsNumber(argument_value)) {
            Argument arg = {0, (int32_t) cJSON_GetNumberValue(argument_value)};
            instruction_args[i] = arg;
        }
        else if (cJSON_IsString(argument_value)) {
            char *address_string = cJSON_GetStringValue(argument_value);
            Argument arg = {1, (int32_t) atoi(address_string+1)};
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
    for (byte i = 0; i < AMOUNT_INSTRUCTIONS; i++) {
        if (strcmp(INSTRUCTIONS[i], instruction_name) == 0) {
            return i;
        }
    }
    return 255;
}


Instruction* parse_instructions_json(cJSON *instructions_json) {
    size_t instruction_count = cJSON_GetArraySize(instructions_json);
    Instruction *instructions = malloc(sizeof(Instruction) * instruction_count);
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
        instructions[i].opcode = opcode;
        byte argument_count = ARGUMENT_COUNTS[opcode];
        int parse_arguments_response = parse_json_arguments(cJSON_GetArrayItem(instruction_data, 1), instructions[i].arguments, argument_count);
        if (parse_arguments_response < 0) {
            free(instructions);
            return NULL;
        }
    }
    return instructions;
}


Instruction* parse_instructions_binary(size_t instruction_count, BytesIterator *iter) {
    Instruction *instructions = malloc(sizeof(Instruction) * instruction_count);
    for (size_t i = 0; i < instruction_count; i++) {
        Instruction *ins = &instructions[i];
        bi_next_n(&ins->opcode, iter, 1);
        size_t argument_count = ARGUMENT_COUNTS[ins->opcode];
        for (size_t j = 0; j < argument_count; j++) {
            Argument arg;
            bi_next_n(&arg.type, iter, 1);
            bi_next_n(&arg.value, iter, 4);
            ins->arguments[j] = arg;
        }
    }

    return instructions;
}