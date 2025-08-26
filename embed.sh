#!/bin/bash

# Script to automatically build an embedded g1 program.

# Initial checks
if [ $# -eq 0 ]; then
    echo "Usage: embed.sh [input path] [output_path] (SHOW_FPS) (PIXEL_SCALE)"
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
xxd -i -n __embedded_program $1 > ../src/cg1/embed.h

# Reconfigure for embed compile
cmake .. -DG1_EMBEDDED=1 -DG1_FLAG_SHOW_FPS=$show_fps_flag -DG1_FLAG_SCALE=$scale_flag

# If `main` already exists, move it temporarily
if [ -f "main" ]; then
    mv main main.old
    moved_main=1
fi

# Compile and move executable
make
cp main $2

# Restore original `main` if necessary
if [ $moved_main -eq 1 ]; then
    mv main.old main
fi

# Reset back to original settings
rm ../src/cg1/embed.h
cmake .. -DG1_EMBEDDED=0 -DG1_FLAG_SHOW_FPS=0 -DG1_FLAG_SCALE=1

exit 0
