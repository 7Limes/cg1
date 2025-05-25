# cg1

An implementation of the [g1](https://github.com/7Limes/g1) virtual machine in C.


### Compiler Options

- `-DENABLE_G1_RUNTIME_ERRORS` (Default: `1`)
  - Controls whether runtime errors will be raised during program execution. If this option is disabled, the program will continue to run even if a runtime error occurs.
  - Disabling this option can cause segfaults. Use at your own risk!
- `-DENABLE_G1_GPU_RENDERING` (Default: `0`)
  - Enable hardware accelerated primitive drawing. Should only be used if the window is being cleared and redrawn each tick.
