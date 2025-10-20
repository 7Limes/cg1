/*
    Flag parsing functions.
*/

#ifndef UTIL_FLAGS_HEADER
#define UTIL_FLAGS_HEADER

#include <stdbool.h>

#define FLAG_BUFFER_SIZE 128
#define TITLE_BUFFER_SIZE 64


struct FlagData {
    bool show_fps;
    uint32_t pixel_size;
    char title[TITLE_BUFFER_SIZE];
};


// Parse flags from a string into a struct.
void parse_flags(struct FlagData* flag_data, const char* flags);


#endif