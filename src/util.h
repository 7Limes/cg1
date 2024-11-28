#include <inttypes.h>
#include "cJSON.h"

typedef unsigned char byte;

// Reads a json file from a file path.
cJSON* json_from_file(const char *file_path);


typedef struct {
    const char *source;
    size_t source_length;
    char delimiter;
    size_t index;
} SplitString;


// A string iterator that returns substrings between a given delimiter.
int ss_new(SplitString *ss, const char *s, char delimiter);

/*
Copies the next substring to `dest`.
Will copy no more than `dest_size` bytes to `dest`.
Return codes:
- `0`: Success
- `-1`: The iterator is finished
- `1`: The length of the next substring was greater than `dest_size`, so only part of the substring was copied to `dest`.
*/
int ss_next(char *dest, SplitString *ss, size_t dest_size);
