#pragma once

#include <cstdint> //For uint8_t/uint16_t
#include <array>
#include <string>
#include <fstream>

struct Chip8 {
    std::array<uint8_t, 4096> memory;  // 4 KB of memory; each element stores 1 byte
    uint16_t pc;                       // Address of next instruction; 16 bits can address all 4 KB (0–4095)

    std::array<uint8_t, 16> V{};       // 16 general-purpose 8-bit registers (V0 to VF)

    void loadROM(const std::string& path);
};