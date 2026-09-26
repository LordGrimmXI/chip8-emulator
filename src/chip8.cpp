#include "chip8.h"

#include <stdexcept> // For std::runtime_error
#include <fstream>

void Chip8::loadROM(const std::string& path) {
    // Open file in binary mode
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open ROM file: " + path);
    }

    file.seekg(0, std::ios::end);
    auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    constexpr std::size_t MAX_ROM_SIZE = 4096 - 0x200; // 4 KB of memory minus the reserved space for the interpreter (0x200)

    if (static_cast<std::size_t>(size) > MAX_ROM_SIZE) {
        throw std::runtime_error("ROM size exceeds available memory: " + std::to_string(size) + " bytes");
    }

    file.read(reinterpret_cast<char*>(&memory[0x200]), size); // Load ROM into memory starting at 0x200
    pc = 0x200; // Set program counter to start of the loaded ROM
}