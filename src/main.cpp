#include <iostream>
#include "chip8.h"

int main(int argc, char** argv) {
    std::cout << "CHIP-8 emulator starting up..." << std::endl;

    Chip8 chip8;
    chip8.loadROM("roms/test.ch8");

    try {
        uint16_t opcode1 = chip8.fetch();
        chip8.execute(opcode1);

        uint16_t opcode2 = chip8.fetch();
        chip8.execute(opcode2);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}