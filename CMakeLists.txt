cmake_minimum_required(VERSION 3.10)

project(chip8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell CMake where SDL2's CMake package is installed.
set(SDL2_PATH "C:/Libraries/SDL2-2.32.10" CACHE PATH "Path to SDL2 dev folder")
set(SDL2_DIR "${SDL2_PATH}/cmake")

# Find SDL2.
find_package(SDL2 CONFIG REQUIRED)

# CHIP-8 emulator
add_executable(chip8
    src/main.cpp
    src/chip8.cpp
)

target_link_libraries(chip8 PRIVATE SDL2::SDL2main SDL2::SDL2)

add_custom_command(TARGET chip8 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "C:/Libraries/SDL2-2.32.10/lib/x64/SDL2.dll"
    $<TARGET_FILE_DIR:chip8>
)

# CHIP-8 tests
add_executable(chip8_tests
    tests/test_chip8.cpp
    src/chip8.cpp
)