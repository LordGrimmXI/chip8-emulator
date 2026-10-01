#include "chip8.h"

#include <stdexcept> // For std::runtime_error
#include <fstream>   // For std::ifstream
#include <sstream>   // For std::ostringstream
#include <algorithm> // For std::fill

std::uniform_int_distribution<int> dist(0, 255); // Distribution for random byte generation

Chip8::Chip8() {
    std::copy(FONT.begin(), FONT.end(), memory.begin() + FONT_START);
}

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

    /*
    Clear the previous ROM from the ROM area of memory before loading a new one.
    Without this, loading a smaller ROM would leave the unused tail of the old ROM
    in memory. For example, if the old ROM occupies 0x200-0x204 but the new ROM
    only occupies 0x200-0x202, addresses 0x203-0x204 would still contain bytes
    from the old ROM. Clearing the ROM area prevents stale data from remaining.
    We clear memory from 0x200 to the end of memory to preserve the font data in the lower memory area (0x000-0x1FF).
    */
    std::fill(memory.begin() + 0x200, memory.end(), 0);

    file.read(reinterpret_cast<char*>(&memory[0x200]), size); // Load ROM into memory starting at 0x200
    pc = 0x200; // Set program counter to start of the loaded ROM
}

uint16_t Chip8::fetch() {
    // Fetch the next instruction from memory at the current program counter (pc)
    uint16_t instruction = (memory[pc] << 8) | memory[pc + 1];
    pc += 2; // Increment program counter to point to the next instruction
    return instruction;
}

void Chip8::execute(uint16_t opcode) {
    uint8_t category = (opcode & 0xF000) >> 12;
    uint8_t X        = (opcode & 0x0F00) >> 8;
    uint8_t Y        = (opcode & 0x00F0) >> 4;
    uint8_t N        = (opcode & 0x000F);
    uint8_t NN       = (opcode & 0x00FF);
    uint16_t NNN     = (opcode & 0x0FFF);

    switch (category) {
        case 0x0:
            if (NN == 0xEE) {
                if (sp == 0) {
                    throw std::runtime_error("Stack underflow: cannot return from subroutine, stack is empty.");
                }
                pc = stack[--sp];
            } else {
                std::ostringstream oss;
                oss << "Unknown opcode: 0x" << std::hex << opcode;
                throw std::runtime_error(oss.str());
            }
            break;

        case 0x1:       // Jump to address NNN
            pc = NNN;
            break;
        
        case 0x2:       // Call subroutine at address NNN
            if (sp >= stack.size()) {
                throw std::runtime_error("Stack overflow: cannot call subroutine, stack is full.");
            }

            stack[sp++] = pc; // Push current pc onto the stack
            pc = NNN;         // Jump to subroutine
            break;

        case 0x3:             // Skip next instruction if VX equals NN
            if (V[X] == NN) {
                pc += 2;
            }
            break;

        case 0x4:             // Skip next instruction if VX does not equal NN
            if (V[X] != NN) {
                pc += 2;
            }
            break;

        case 0x5:             // Skip next instruction if VX equals VY
            if (V[X] == V[Y]) {
                pc += 2;
            }
            break;

        case 0x6:             // Set VX to NN
            V[X] = NN;
            break;
        
        case 0x7:             // Add NN to VX (without carry)
            // V[X] is 8-bit, so overflow naturally wraps around.
            // This is intentional CHIP-8 behavior; VF is NOT affected by 7XNN.
            V[X] += NN;
            break;
        
        case 0x8:             // Arithmetic and logic operations between VX and VY
            switch(N) {
                case 0:
                    V[X] = V[Y];  // Set VX to the value of VY
                    break;
                
                case 1:
                    V[X] |= V[Y]; // Set VX to VX OR VY
                    break;

                case 2:
                    V[X] &= V[Y]; // Set VX to VX AND VY
                    break;
                
                case 3:
                    V[X] ^= V[Y]; // Set VX to VX XOR VY
                    break;
                
                case 4: {
                    uint16_t sum = static_cast<uint16_t>(V[X]) + V[Y];

                    V[0xF] = (sum > 0xFF) ? 1 : 0;
                    V[X] = static_cast<uint8_t>(sum); // Set VX to VX + VY; VF is set if carry occurs
                    
                    break;
                }

                case 5: {
                    uint8_t noBorrow = (V[X] >= V[Y]) ? 1 : 0;
                    
                    V[X] -= V[Y]; // Set VX to VX - VY; VF is set if no borrow occurs
                    V[0xF] = noBorrow;
                    
                    break;
                }

                case 6: {
                    uint8_t shiftedOut = V[X] & 0x01;
                    
                    V[X] >>= 1; // Shift VX right by 1; VF gets the bit shifted out
                    V[0xF] = shiftedOut;
                    
                    break;
                }

                case 7: {
                    uint8_t noBorrow = (V[Y] >= V[X]) ? 1 : 0;
                    
                    V[X] = V[Y] - V[X]; // Set VX to VY - VX; VF is set if no borrow occurs
                    V[0xF] = noBorrow;
                    
                    break;
                }

                case 0xE: {
                    uint8_t shiftedOut = (V[X] & 0x80) >> 7;
                    
                    V[X] <<= 1; // Shift VX left by 1; VF gets the bit shifted out
                    V[0xF] = shiftedOut;
                    
                    break;
                }

                default: {
                    std::ostringstream oss;
                    oss << "Unknown opcode: 0x" << std::hex << opcode;
                    throw std::runtime_error(oss.str());
                }
            }
            break;

        case 0x9:             // Skip next instruction if VX does not equal VY
            if (V[X] != V[Y]) {
                pc += 2;
            }
            break;

        case 0xA:             // Set I to the address NNN
            I = NNN;
            break;

        case 0xC: {                         // Set VX to a random byte AND NN
            uint8_t randomByte = static_cast<uint8_t>(dist(rng)); // Generate a random byte
            V[X] = randomByte & NN;         // Set VX to the result of the AND operation
            break;
        }

        case 0xF:
            switch(NN) {
                case 0x1E:
                    I += V[X];
                    break;

                case 0x29:
                    I = FONT_START + (V[X] * 5);
                    break;

                default: {
                    std::ostringstream oss;
                    oss << "Unknown opcode: 0x" << std::hex << opcode;
                    throw std::runtime_error(oss.str());
                }
            }
            break;

        default: {
            std::ostringstream oss;
            oss << "Unknown opcode: 0x" << std::hex << opcode;
            throw std::runtime_error(oss.str());
        }
    }
}