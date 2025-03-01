#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include "cg1.h"


#define FLAG_BUFFER_SIZE 128


bool file_exists(char* path) {
    struct stat buffer;
    return stat(path, &buffer) == 0;
}

bool safecat(char* dest, char* src, int size) {
    if (strlen(dest) + strlen(src) >= size)
        return false;
    strncat(dest, src, size-1);  // size-1 to avoid compiler warning on O2
    return true;
}

int main(int argc, char* argv[]) {
    char flags[FLAG_BUFFER_SIZE];
    if (argc == 1) {
        printf("Usage: [program path] (flags)\n");
        return 1;
    }
    else if (argc > 2) {
        for (int i = 2; i < argc; i++) {
            bool result = safecat(flags, argv[i], FLAG_BUFFER_SIZE);
            if (i != argc-1)
                result = result && safecat(flags, " ", FLAG_BUFFER_SIZE);
            if (!result) {
                printf("Flag buffer overflow at index %d.\n", i);
                return 2;
            }
        }
    }
    if (!file_exists(argv[1])) {
        printf("File \"%s\" does not exist.", argv[1]);
        return 3;
    }

    run_file(argv[1], flags);
    return 0;
}