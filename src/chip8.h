#pragma once

#include <cstdint> //For uint8_t/uint16_t
#include <array>
#include <string>
#include <fstream>
#include <random>

struct Chip8 {
    std::array<uint8_t, 4096> memory{};       // 4 KB of memory; each element stores 1 byte
    uint16_t pc{0x200};                       // Address of next instruction; 16 bits can address all 4 KB Memory Address(0–4095)
    uint16_t I{0};                            // Index register; same as pc, needs only 12 bits so uint16_t is more than enough

    std::array<uint8_t, 16> V{};              // 16 general-purpose 8-bit registers (V0 to VF)

    std::array<uint16_t, 16> stack{};         // Stack for storing return addresses; 16 levels deep
    uint8_t sp{};                             // Stack pointer; points to the top of the stack

    std::array<bool, 16> keys{};              // Input keys; 16 keys (0x0 to 0xF)

    static constexpr int DISPLAY_WIDTH  = 64;
    static constexpr int DISPLAY_HEIGHT = 32;

    std::array<uint8_t, DISPLAY_WIDTH * DISPLAY_HEIGHT> display{};    // Display buffer; 64x32 pixels, each pixel is either on (1) or off (0)
    bool draw_flag = false;

    std::mt19937 rng{std::random_device{}()}; // Mersenne Twister random number generator

    void loadROM(const std::string& path);

    uint16_t fetch();

    void execute(uint16_t opcode);

    static constexpr std::array<uint8_t, 80> FONT = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    static constexpr uint16_t FONT_START = 0x50;

    Chip8();
};