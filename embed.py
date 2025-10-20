import argparse 
import os
import subprocess
import shutil


BUILD_DIRECTORY = 'embed_build'
EMBEDDED_VAR_NAME = '__embedded_program'
EMBED_H_PATH = 'src/cg1/embed.h'
OUTPUT_EXECUTABLE_NAME = 'main'

CMAKE_BASE_COMMAND = ['cmake', '-B', BUILD_DIRECTORY, '-DG1_EMBEDDED=ON']

MINGW_TOOLCHAIN_PATH = 'mingw-w64-toolchain.cmake'


def build(input_path: str, output_path: str, show_fps: bool, scale: int, title: str, static: bool, windows: bool):
    if not os.path.isfile(input_path):
        raise FileNotFoundError(f'Could not find file "{input_path}"')
    
    # Create embed.h
    xxd_command = ['xxd', '-i', '-n', EMBEDDED_VAR_NAME, input_path]
    xxd_result = subprocess.run(xxd_command, capture_output=True, text=True)
    with open(EMBED_H_PATH, 'w') as f:
        f.write(xxd_result.stdout)

    # Create cmake command
    cmake_command = CMAKE_BASE_COMMAND
    cmake_command.extend([f'-DG1_FLAG_SHOW_FPS={show_fps}', f'-DG1_FLAG_SCALE={scale}', f'-DG1_FLAG_TITLE={title}'])

    if static:
        cmake_command.append('-DSTATIC_BUILD=ON')
    
    if windows:
        cmake_command.append(f'-DCMAKE_TOOLCHAIN_FILE={MINGW_TOOLCHAIN_PATH}')

    # Clear old build directory
    if os.path.exists(BUILD_DIRECTORY):
        shutil.rmtree(BUILD_DIRECTORY)
    
    # Configure and build
    subprocess.run(cmake_command)
    subprocess.run(['cmake', '--build', BUILD_DIRECTORY])

    # Copy the built executable to the output path
    output_executable = OUTPUT_EXECUTABLE_NAME
    if windows:
        output_executable += '.exe'
    shutil.copy(f'{BUILD_DIRECTORY}/{output_executable}', output_path)

    # Remove embed.h
    os.remove(EMBED_H_PATH)


def main():
    parser = argparse.ArgumentParser('embed', description='Automatically build an embedded g1 program')
    parser.add_argument('input_path', type=str, help='The .g1b program to embed')
    parser.add_argument('output_path', type=str, help='The path to the output executable')
    parser.add_argument('--show_fps', '-f', action='store_true')
    parser.add_argument('--scale', '-s', default=1)
    parser.add_argument('--title', '-t', type=str, default='cg1', help='The title of the output window')
    parser.add_argument('--static', '-st', action='store_false', help='Enable static linking')
    parser.add_argument('--windows', '-win', action='store_true', help='Build for Windows')

    args = parser.parse_args()

    try:
        build(args.input_path, args.output_path, args.show_fps, args.scale, args.title, args.static, args.windows)
    except FileNotFoundError as e:
        print(e)
        return 1
    
    print('Build successful.')
    return 0


if __name__ == '__main__':
    main()
