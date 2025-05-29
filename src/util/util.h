/*
    Utility functions and structures.
*/

#ifndef UTIL_HEADER
#define UTIL_HEADER

#include <inttypes.h>
#include <stdbool.h>
#include "cJSON.h"


typedef unsigned char byte;


// Iterator over a string split at each occurrence of a character
typedef struct {
    const char *source;
    size_t source_length;
    char delimiter;
    size_t index;
} SplitString;


// Iterator over a group of bytes
typedef struct {
    const byte *bytes;
    size_t length, index;
} BytesIterator;


// Returns true if a file exists at the specified path.
bool file_exists(char* path);


/*
Reads data from `file_path` into `output_buffer` and stores the length in `length`.
Return codes:
- `0` - Success
- `-1` - Invalid arguments
- `-2` - Error opening file
- `-3` - Error seeking file
- `-4` - Error getting file size
- `-5` - Error allocating output buffer
- `-6` - Error reading file

Based on https://stackoverflow.com/a/3464656
*/
int read_file_bytes(byte **output_buffer, size_t *length, const char *file_path);

// Reads a json file from a file path.
cJSON* json_from_file(const char *file_path);


// Concatenate `src` onto `dest` with a maximum of `size` bytes.
bool safecat(char* dest, char* src, int size);


// Create a new SplitString
void ss_new(SplitString *ss, const char *s, char delimiter);

/*
Copies the next substring to `dest`.
Will copy no more than `dest_size` bytes to `dest`.
Return codes:
- `0`: Success
- `-1`: The iterator is finished
- `1`: The length of the next substring was greater than `dest_size`, so only part of the substring was copied to `dest`.
*/
int ss_next(char *dest, SplitString *ss, size_t dest_size);


// Create a new `BytesIterator`
void bi_new(BytesIterator *iter, char *bytes, size_t length);

/*
Copy the next `n` bytes of `iter` to `dest`.
Return codes:
- `0`: Success
- `-1`: Iterator went out of bounds
- `1`: The iterator is finished
*/
int bi_next_n(void *dest, BytesIterator *iter, size_t n);

#endif