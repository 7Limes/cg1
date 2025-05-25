cmake_minimum_required(VERSION 3.10)
project(MyProject)

# Find the SDL2 package
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)

# Include directories for headers
include_directories(
    ${SDL2_INCLUDE_DIR}
    src/cg1
    src/cjson
    src/instruction
    src/program
    src/util
    src/render
    src/font
)

# Specify sources
set(SOURCES
    src/main.c
    src/cg1/cg1.c
    src/cjson/cJSON.c
    src/instruction/instruction.c
    src/program/program.c
    src/util/util.c
    src/render/cpu_primitives.c
)

# Option to enable runtime errors
option(ENABLE_G1_RUNTIME_ERRORS "Enable runtime errors" ON)
if(ENABLE_G1_RUNTIME_ERRORS)
    add_definitions(-DENABLE_G1_RUNTIME_ERRORS)
endif()

# Option to enable GPU rendering
option(ENABLE_G1_GPU_RENDERING "Enable GPU rendering" OFF)
if(ENABLE_G1_GPU_RENDERING)
    add_definitions(-DENABLE_G1_GPU_RENDERING)
endif()

# Set the build type for optimization (-O2 equivalent)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Create an executable target
add_executable(main ${SOURCES})

# Link against SDL2
target_link_libraries(main ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES})

# Set optimization flags
set_target_properties(main PROPERTIES COMPILE_FLAGS "-O2")
