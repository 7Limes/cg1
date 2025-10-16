# cg1 Compile Options

## Build Options
- `-DSTATIC_BUILD` (Default: `OFF`)
  - Statically link libraries on build.
- `-DG1_EMBEDDED` (Default: `OFF`)
  - Build the virtual machine with an embedded program.
  - The compiled executable will automatically run the embedded program, so this option is good for standalone apps.
  - Requires an `embed.h` file containing the `xxd` output of a `.g1b` program to be placed in `src/cg1`.

## g1 Options
- `-DENABLE_G1_RUNTIME_ERRORS` (Default: `ON`)
  - Controls whether runtime errors will be raised during program execution. If this option is disabled, the program will continue to run even if a runtime error occurs.
  - Disabling this option can cause segfaults. Use at your own risk!
- `-DENABLE_G1_LOG_INSTRUCTION` (Default: `ON`)
  - Whether the log instruction should print to stdout.
- `-DENABLE_G1_GPU_RENDERING` (Default: `OFF`)
  - Enable hardware accelerated primitive drawing. Should only be used if the window is being cleared and redrawn each tick.

## g1 Flags (EMBEDDED ONLY)

These variables will only be used if `-DG1_EMBEDDED` is set.

- `-DG1_FLAG_SHOW_FPS` (Default: `OFF`)
  - Whether the framerate label should be displayed at runtime.
- `-DG1_FLAG_SCALE` (Default: `1`)
  - The scale factor to use for the output buffer.
