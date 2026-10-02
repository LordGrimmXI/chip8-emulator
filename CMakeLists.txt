cmake_minimum_required(VERSION 3.10)
project(chip8)

# CHIP-8 project uses plain C++17 — no external dependencies yet.
# SDL2 gets added here on Day 2, once we build the display.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Every .cpp file that's part of the emulator goes in this list.
# As you add chip8.cpp's implementation and (later) display.cpp,
# just add the filename below.
add_executable(chip8
    src/main.cpp
    src/chip8.cpp
)

add_executable(chip8_tests
    tests/test_chip8.cpp
    src/chip8.cpp
)