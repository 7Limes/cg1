/*
    Flag parsing functions.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

#define FLAG_BUFFER_SIZE 128


struct FlagData {
    bool show_fps;
    uint32_t pixel_size;
};


void parse_flags(struct FlagData* flag_data, const char* flags) {
    flag_data->show_fps = false;
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
        else {
            printf("Unrecognized flag \"%s\".\n", flag_buffer);
        }
    }
}