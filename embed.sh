#!/bin/bash

# Script to automatically build an embedded g1 program.

# Initial checks
if [ $# -eq 0 ]; then
    echo "Usage: embed.sh [input path (.g1b)] [output_path] (SHOW_FPS) (PIXEL_SCALE)"
    exit 1
fi

if [ ! -f $1 ]; then
    echo "Could not find file \"$1\""
    exit 2
fi


# Assign flag variables
if [ -z $3 ]; then
    show_fps_flag=0
else
    show_fps_flag=$3
fi

if [ -z $4 ]; then
    scale_flag=1
else
    scale_flag=$4
fi


# Create embed.h
xxd -i -n __embedded_program $1 > src/cg1/embed.h

# Reconfigure for embed compile
cmake -B embed_build -DG1_EMBEDDED=ON -DSTATIC_BUILD=ON -DG1_FLAG_SHOW_FPS=$show_fps_flag -DG1_FLAG_SCALE=$scale_flag

# Compile and move executable
cmake --build embed_build
cp embed_build/main $2

# Remove embed file
rm src/cg1/embed.h

exit 0
