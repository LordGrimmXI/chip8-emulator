#pragma once

#include <cstdint> //For uint8_t/uint16_t
#include <array>
#include <string>
#include <fstream>

struct Chip8 {
    std::array<uint8_t, 4096> memory;       // 4 KB of memory; each element stores 1 byte
    uint16_t pc{0x200};                     // Address of next instruction; 16 bits can address all 4 KB Memory Address(0–4095)
    uint16_t I{0};                          // Index register; same as pc, needs only 12 bits so uint16_t is more than enough

    std::array<uint8_t, 16> V{};            // 16 general-purpose 8-bit registers (V0 to VF)

    std::array<uint16_t, 16> stack{};       // Stack for storing return addresses; 16 levels deep
    uint8_t sp{};                           // Stack pointer; points to the top of the stack

    std::array<bool, 16> keys{};            // Input keys; 16 keys (0x0 to 0xF)
    std::array<uint8_t, 64 * 32> display{}; // Display buffer; 64x32 pixels, each pixel is either on (1) or off (0)

    void loadROM(const std::string& path);
};