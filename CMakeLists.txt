cmake_minimum_required(VERSION 3.10)
project(MyProject)

# Option to enable static linking
option(STATIC_BUILD "Build with static linking" OFF)

# Detect if we're cross-compiling for Windows with MinGW
if(WIN32 OR MINGW)
    set(WINDOWS_BUILD TRUE)
    message(STATUS "Configuring for Windows build (MinGW)")
else()
    set(WINDOWS_BUILD FALSE)
endif()

# Find the SDL2 package
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)

# Include directories for headers
include_directories(
    ${SDL2_INCLUDE_DIRS}
    src/cg1
    src/cjson
    src/instruction
    src/program
    src/util
    src/render
    src/font
    src/audio
)

# Specify sources
set(SOURCES
    src/main.c
    src/cg1/cg1.c
    src/cjson/cJSON.c
    src/instruction/instruction.c
    src/program/program.c
    src/util/util.c
    src/util/flags.c    
    src/audio/audio.c
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

# Option for embedded program
option(G1_EMBEDDED "Compile with an embedded program" OFF)
if(G1_EMBEDDED)
    add_definitions(-DG1_EMBEDDED)
endif()

# Flags for embedded programs
option(G1_FLAG_SHOW_FPS "(EMBEDDED ONLY) Whether fps should be shown at runtime")
if(G1_FLAG_SHOW_FPS)
    add_definitions(-DG1_FLAG_SHOW_FPS=1)
else()
    add_definitions(-DG1_FLAG_SHOW_FPS=0)
endif()

set(G1_FLAG_SCALE 1 CACHE STRING "(EMBEDDED ONLY) The value by which to scale the final output")
if(NOT G1_FLAG_SCALE MATCHES "^[0-9]+$")
    message(FATAL_ERROR "G1_FLAG_SCALE must be a positive integer")
else()
    add_definitions(-DG1_FLAG_SCALE=${G1_FLAG_SCALE})
endif()

set(G1_FLAG_TITLE "cg1" CACHE STRING "(EMBEDDED ONLY) The title of the output window")
add_definitions(-DG1_FLAG_TITLE="${G1_FLAG_TITLE}")

# Set the build type for optimization (-O2 equivalent)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Create an executable target
add_executable(main ${SOURCES})

if(WINDOWS_BUILD AND CMAKE_BUILD_TYPE STREQUAL "Release")
    # Hide the console window on Windows
    target_link_options(main PRIVATE -mwindows)
endif()

# Configure linking based on platform and STATIC_BUILD option
if(STATIC_BUILD)
    if(WINDOWS_BUILD)
        # Windows static linking with MinGW
        execute_process(
            COMMAND ${PKG_CONFIG_EXECUTABLE} --libs --static sdl2
            OUTPUT_VARIABLE SDL2_STATIC_LDFLAGS
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        execute_process(
            COMMAND ${PKG_CONFIG_EXECUTABLE} --libs --static SDL2_ttf
            OUTPUT_VARIABLE SDL2_TTF_STATIC_LDFLAGS
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        
        target_link_libraries(main ${SDL2_STATIC_LDFLAGS} ${SDL2_TTF_STATIC_LDFLAGS})
        
        # Add additional Windows static libraries if needed
        target_link_options(main PRIVATE 
            -static-libgcc 
            -static-libstdc++ 
            -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive
        )
        
        message(STATUS "Building for Windows with static linking")
        message(STATUS "SDL2 static flags: ${SDL2_STATIC_LDFLAGS}")
        message(STATUS "SDL2_ttf static flags: ${SDL2_TTF_STATIC_LDFLAGS}")
    else()
        # Linux static linking
        execute_process(
            COMMAND pkg-config --libs --static sdl2
            OUTPUT_VARIABLE SDL2_STATIC_LDFLAGS
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        execute_process(
            COMMAND pkg-config --libs --static SDL2_ttf
            OUTPUT_VARIABLE SDL2_TTF_STATIC_LDFLAGS
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        
        target_link_libraries(main ${SDL2_STATIC_LDFLAGS} ${SDL2_TTF_STATIC_LDFLAGS})
        target_link_options(main PRIVATE 
            -static-libgcc 
            -static-libstdc++ 
            -Wl,-Bstatic,--whole-archive -lSDL2 -lSDL2_ttf -Wl,-Bdynamic,--no-whole-archive
        )
        
        message(STATUS "Building for Linux with static linking")
        message(STATUS "SDL2 static flags: ${SDL2_STATIC_LDFLAGS}")
        message(STATUS "SDL2_ttf static flags: ${SDL2_TTF_STATIC_LDFLAGS}")
    endif()
else()
    # Dynamic linking
    target_link_libraries(main ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES})
    if(WINDOWS_BUILD)
        message(STATUS "Building for Windows with dynamic linking")
    else()
        message(STATUS "Building for Linux with dynamic linking")
    endif()
endif()

# Set optimization flags
set_target_properties(main PROPERTIES COMPILE_FLAGS "-O2")