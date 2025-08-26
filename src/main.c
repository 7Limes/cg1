#include <stdio.h>
#include "util.h"
#include "flags.h"
#include "cg1.h"


int main_cli(int argc, char* argv[]) {
    char flags[FLAG_BUFFER_SIZE] = "";
    if (argc == 1) {
        printf("usage: cg1 [--show_fps] [--scale SCALE] program_path\n");
        return 1;
    }
    
    if (argc > 2) {
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
        printf("File \"%s\" does not exist.\n", argv[1]);
        return 3;
    }

    run_file(argv[1], flags);
    return 0;
}


int main(int argc, char* argv[]) {
    #ifdef G1_EMBEDDED
        run_embedded();
    #else
        return main_cli(argc, argv);
    #endif
}