/*
    Utility functions and structures.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "cJSON.h"

typedef unsigned char byte;


// Based on https://stackoverflow.com/a/3464656
// Reads data from `file_path` into `output_buffer` and stores the length in `length`.
int read_file_bytes(byte **output_buffer, size_t *length, const char *file_path) {
    if (!output_buffer || !length || !file_path) {
        return -1;  // Invalid arguments
    }

    FILE *handler = fopen(file_path, "rb");
    if (!handler) {
        return -2;  // File cannot be opened
    }
    
    if (fseek(handler, 0, SEEK_END) != 0) {
        fclose(handler);
        return -3;  // Seek error
    }

    long file_size = ftell(handler);
    if (file_size < 0) {
        fclose(handler);
        return -4;  // Tell error
    }
    rewind(handler);

    byte *buffer = malloc((size_t)file_size + 1);
    if (!buffer) {
        fclose(handler);
        return -5;  // Memory allocation failure
    }

    size_t read_size = fread(buffer, sizeof (byte), (size_t) file_size, handler);
    if (read_size != (size_t) file_size) {
        free(buffer);
        fclose(handler);
        return -6;  // Read error
    }

    buffer[file_size] = '\0';
    fclose(handler);

    *output_buffer = buffer;
    *length = (size_t) file_size;

    return 0;  // Success
}


cJSON* json_from_file(const char *file_path) {
    byte *file_bytes;
    size_t file_bytes_size;
    int read_file_response = read_file_bytes(&file_bytes, &file_bytes_size, file_path);
    if (read_file_response != 0) {
        return NULL;
    }
    cJSON *json = cJSON_Parse(file_bytes);
    free(file_bytes);
    if (!json) {
        return NULL;
    }
    return json;
}


// Iterator over a string split at each occurrence of a character
typedef struct {
    const char *source;
    size_t source_length;
    char delimiter;
    size_t index;
} SplitString;


void ss_new(SplitString *ss, const char *string, char delimiter) {
    ss->source = string;
    ss->source_length = strlen(string);
    ss->delimiter = delimiter;
    ss->index = 0;
}


int ss_next(char *dest, SplitString *ss, size_t dest_size) {
    if (ss->index == ss->source_length+1) {
        return -1;  // Iterator is finished
    }

    size_t delimiter_index = SIZE_MAX;
    for (size_t i = ss->index; i < ss->source_length; i++) {
        if (ss->source[i] == ss->delimiter) {
            delimiter_index = i;
            break;
        }
    }
    if (delimiter_index == SIZE_MAX) {
        delimiter_index = ss->source_length;
    }
    
    size_t substring_length = delimiter_index-ss->index;
    int return_code = substring_length > dest_size;
    substring_length = substring_length > dest_size ? dest_size : substring_length;

    dest[0] = '\0';  // Set first character to null terminator so `strncat` just copies.
    strncat(dest, ss->source+ss->index, substring_length);
    ss->index = delimiter_index+1;
    return return_code;
}


int is_little_endian() {
    byte swaptest[2] = {1, 0};
    return *(short *)swaptest == 1;
}


// Same as memcpy, but copies bytes in reverse order.
void* memcpy_reverse(void *restrict dest, const void *restrict src, size_t n) {
    byte* dest_p = (byte*)dest;
    const byte* src_p = (const byte*)src;

    for (size_t i = 0; i < n; i++) {
        dest_p[i] = src_p[n - 1 - i];
    }

    return dest;
}


// Iterator over a group of bytes.
typedef struct {
    byte *bytes;
    size_t length, index;
} BytesIterator;


void bi_new(BytesIterator *iter, char *bytes, size_t length) {
    iter->bytes = bytes;
    iter->length = length;
    iter->index = 0;
}


int bi_next_n(void *dest, BytesIterator *iter, size_t n) {
    if (n + iter->index > iter->length) {
        return -1;
    }

    if (dest != NULL) {
        if (is_little_endian()) {
            memcpy_reverse(dest, iter->bytes+iter->index, n);
        }
        else {
            memcpy(dest, iter->bytes+iter->index, n);
        }
    }
    iter->index += n;
    return iter->length == iter->index;
}