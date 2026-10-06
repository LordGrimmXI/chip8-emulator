#include "../src/chip8.h"

#include <cassert>
#include <iostream>
#include <stdexcept>

// ------------------------------------------------------------
// 0x0NNN / 00EE
// ------------------------------------------------------------

void test_return() {
    Chip8 c;

    c.pc = 0x400;
    c.sp = 1;
    c.stack[0] = 0x300;

    c.execute(0x00EE);

    assert(c.pc == 0x300);
    assert(c.sp == 0);
}

void test_return_stack_underflow() {
    Chip8 c;

    c.sp = 0;

    bool threw = false;

    try {
        c.execute(0x00EE);
    }
    catch (const std::runtime_error&) {
        threw = true;
    }

    assert(threw);
}

// ------------------------------------------------------------
// 1NNN - Jump
// ------------------------------------------------------------

void test_jump() {
    Chip8 c;

    c.execute(0x1300);

    assert(c.pc == 0x300);
}

// ------------------------------------------------------------
// 2NNN - Call subroutine
// ------------------------------------------------------------

void test_call_and_return() {
    Chip8 c;

    c.pc = 0x200;

    c.execute(0x2300);

    assert(c.pc == 0x300);
    assert(c.sp == 1);
    assert(c.stack[0] == 0x200);

    c.execute(0x00EE);

    assert(c.pc == 0x200);
    assert(c.sp == 0);
}

void test_call_stack_overflow() {
    Chip8 c;

    c.sp = static_cast<uint8_t>(c.stack.size());
    c.pc = 0x200;

    bool threw = false;

    try {
        c.execute(0x2300);
    }
    catch (const std::runtime_error&) {
        threw = true;
    }

    assert(threw);
}

// ------------------------------------------------------------
// 3XNN - Skip if VX == NN
// ------------------------------------------------------------

void test_skip_if_equal() {
    Chip8 c;

    c.pc = 0x200;
    c.V[3] = 0x42;

    c.execute(0x3342);

    assert(c.pc == 0x202);
}

void test_skip_if_equal_not_taken() {
    Chip8 c;

    c.pc = 0x200;
    c.V[3] = 0x41;

    c.execute(0x3342);

    assert(c.pc == 0x200);
}

// ------------------------------------------------------------
// 4XNN - Skip if VX != NN
// ------------------------------------------------------------

void test_skip_if_not_equal() {
    Chip8 c;

    c.pc = 0x200;
    c.V[3] = 0x41;

    c.execute(0x4342);

    assert(c.pc == 0x202);
}

void test_skip_if_not_equal_not_taken() {
    Chip8 c;

    c.pc = 0x200;
    c.V[3] = 0x42;

    c.execute(0x4342);

    assert(c.pc == 0x200);
}

// ------------------------------------------------------------
// 5XY0 - Skip if VX == VY
// ------------------------------------------------------------

void test_skip_if_registers_equal() {
    Chip8 c;

    c.pc = 0x200;
    c.V[3] = 10;
    c.V[4] = 10;

    c.execute(0x5340);

    assert(c.pc == 0x202);
}

void test_skip_if_registers_not_equal() {
    Chip8 c;

    c.pc = 0x200;
    c.V[3] = 10;
    c.V[4] = 20;

    c.execute(0x5340);

    assert(c.pc == 0x200);
}

// ------------------------------------------------------------
// 6XNN - Set VX
// ------------------------------------------------------------

void test_set_register() {
    Chip8 c;

    c.V[3] = 0;

    c.execute(0x63AB);

    assert(c.V[3] == 0xAB);
}

// ------------------------------------------------------------
// 7XNN - Add NN to VX
// ------------------------------------------------------------

void test_add_value_to_register() {
    Chip8 c;

    c.V[3] = 10;

    c.execute(0x7305);

    assert(c.V[3] == 15);
}

void test_add_value_wraps() {
    Chip8 c;

    c.V[3] = 250;

    c.execute(0x730A);

    // 250 + 10 = 260 -> 4 in an 8-bit register
    assert(c.V[3] == 4);
}

// ------------------------------------------------------------
// 8XY0 - VX = VY
// ------------------------------------------------------------

void test_register_move() {
    Chip8 c;

    c.V[3] = 0;
    c.V[4] = 0xAB;

    c.execute(0x8340);

    assert(c.V[3] == 0xAB);
}

// ------------------------------------------------------------
// 8XY1 - VX |= VY
// ------------------------------------------------------------

void test_or() {
    Chip8 c;

    c.V[3] = 0b10100000;
    c.V[4] = 0b00001111;

    c.execute(0x8341);

    assert(c.V[3] == 0b10101111);
}

// ------------------------------------------------------------
// 8XY2 - VX &= VY
// ------------------------------------------------------------

void test_and() {
    Chip8 c;

    c.V[3] = 0b10101111;
    c.V[4] = 0b00001111;

    c.execute(0x8342);

    assert(c.V[3] == 0b00001111);
}

// ------------------------------------------------------------
// 8XY3 - VX ^= VY
// ------------------------------------------------------------

void test_xor() {
    Chip8 c;

    c.V[3] = 0b10101111;
    c.V[4] = 0b00001111;

    c.execute(0x8343);

    assert(c.V[3] == 0b10100000);
}

// ------------------------------------------------------------
// 8XY4 - VX += VY, VF = carry
// ------------------------------------------------------------

void test_add_registers_with_carry() {
    Chip8 c;

    c.V[3] = 200;
    c.V[4] = 100;

    c.execute(0x8344);

    assert(c.V[3] == 44);
    assert(c.V[0xF] == 1);
}

void test_add_registers_without_carry() {
    Chip8 c;

    c.V[3] = 100;
    c.V[4] = 50;

    c.execute(0x8344);

    assert(c.V[3] == 150);
    assert(c.V[0xF] == 0);
}

void test_8XY4_flag_wins_when_X_is_VF() {
    Chip8 c;
    c.V[0xF] = 0xFF;
    c.V[1] = 0x01;
    c.execute(0x8F14);          // VF = VF + V1; sum = 0x100, so carry
    assert(c.V[0xF] == 1);      // the flag, not the truncated sum (0x00)
}

// ------------------------------------------------------------
// 8XY5 - VX -= VY, VF = no borrow
// ------------------------------------------------------------

void test_subtract_registers_without_borrow() {
    Chip8 c;

    c.V[3] = 100;
    c.V[4] = 40;

    c.execute(0x8345);

    assert(c.V[3] == 60);
    assert(c.V[0xF] == 1);
}

void test_subtract_registers_with_borrow() {
    Chip8 c;

    c.V[3] = 40;
    c.V[4] = 100;

    c.execute(0x8345);

    assert(c.V[3] == 196);
    assert(c.V[0xF] == 0);
}

// ------------------------------------------------------------
// 8XY6 - Shift VX right
// ------------------------------------------------------------

void test_shift_right() {
    Chip8 c;

    c.V[3] = 0b00000101;

    c.execute(0x8346);

    assert(c.V[3] == 0b00000010);
    assert(c.V[0xF] == 1);
}

void test_shift_right_zero_bit() {
    Chip8 c;

    c.V[3] = 0b00000100;

    c.execute(0x8346);

    assert(c.V[3] == 0b00000010);
    assert(c.V[0xF] == 0);
}

// ------------------------------------------------------------
// 8XY7 - VX = VY - VX, VF = no borrow
// ------------------------------------------------------------

void test_reverse_subtract_without_borrow() {
    Chip8 c;

    c.V[3] = 40;
    c.V[4] = 100;

    c.execute(0x8347);

    assert(c.V[3] == 60);
    assert(c.V[0xF] == 1);
}

void test_reverse_subtract_with_borrow() {
    Chip8 c;

    c.V[3] = 100;
    c.V[4] = 40;

    c.execute(0x8347);

    assert(c.V[3] == 196);
    assert(c.V[0xF] == 0);
}

// ------------------------------------------------------------
// 8XYE - Shift VX left
// ------------------------------------------------------------

void test_shift_left() {
    Chip8 c;

    c.V[3] = 0b10000001;

    c.execute(0x834E);

    assert(c.V[3] == 0b00000010);
    assert(c.V[0xF] == 1);
}

void test_shift_left_zero_bit() {
    Chip8 c;

    c.V[3] = 0b01000000;

    c.execute(0x834E);

    assert(c.V[3] == 0b10000000);
    assert(c.V[0xF] == 0);
}

// ------------------------------------------------------------
// 9XY0 - Skip if VX != VY
// ------------------------------------------------------------

void test_skip_if_registers_not_equal_9() {
    Chip8 c;

    c.pc = 0x200;
    c.V[3] = 10;
    c.V[4] = 20;

    c.execute(0x9340);

    assert(c.pc == 0x202);
}

void test_skip_if_registers_equal_9() {
    Chip8 c;

    c.pc = 0x200;
    c.V[3] = 10;
    c.V[4] = 10;

    c.execute(0x9340);

    assert(c.pc == 0x200);
}

// ------------------------------------------------------------
// ANNN - Set I
// ------------------------------------------------------------

void test_set_I() {
    Chip8 c;

    c.execute(0xA345);

    assert(c.I == 0x345);
}

// ------------------------------------------------------------
// CXNN - Random byte & NN
// ------------------------------------------------------------

void test_random_and() {
    Chip8 c;

    c.execute(0xC3F0);

    // Result must only contain bits that exist in 0xF0.
    assert((c.V[3] & 0x0F) == 0);
}

// ------------------------------------------------------------
// FX1E - I += VX
// ------------------------------------------------------------

void test_add_VX_to_I() {
    Chip8 c;

    c.I = 0x300;
    c.V[3] = 0x20;

    c.execute(0xF31E);

    assert(c.I == 0x320);
}

// ------------------------------------------------------------
// FX29 - Point I to font sprite
// ------------------------------------------------------------

void test_font_address() {
    Chip8 c;

    c.V[3] = 0xA;

    c.execute(0xF329);

    assert(c.I == 0x50 + (0xA * 5));
}

// ------------------------------------------------------------
// FX33 - BCD conversion
// ------------------------------------------------------------

void test_bcd() {
    Chip8 c;

    c.I = 0x300;
    c.V[3] = 123;

    c.execute(0xF333);

    assert(c.memory[0x300] == 1);
    assert(c.memory[0x301] == 2);
    assert(c.memory[0x302] == 3);
}

void test_bcd_with_leading_zero() {
    Chip8 c;

    c.I = 0x300;
    c.V[3] = 5;

    c.execute(0xF333);

    assert(c.memory[0x300] == 0);
    assert(c.memory[0x301] == 0);
    assert(c.memory[0x302] == 5);
}

// ------------------------------------------------------------
// FX55 - Store V0 through VX
// ------------------------------------------------------------

void test_store_registers() {
    Chip8 c;

    c.I = 0x300;

    c.V[0] = 10;
    c.V[1] = 20;
    c.V[2] = 30;
    c.V[3] = 40;

    c.execute(0xF355);

    assert(c.memory[0x300] == 10);
    assert(c.memory[0x301] == 20);
    assert(c.memory[0x302] == 30);
    assert(c.memory[0x303] == 40);
}

// ------------------------------------------------------------
// FX65 - Load V0 through VX
// ------------------------------------------------------------

void test_load_registers() {
    Chip8 c;

    c.I = 0x300;

    c.memory[0x300] = 10;
    c.memory[0x301] = 20;
    c.memory[0x302] = 30;
    c.memory[0x303] = 40;

    c.execute(0xF365);

    assert(c.V[0] == 10);
    assert(c.V[1] == 20);
    assert(c.V[2] == 30);
    assert(c.V[3] == 40);
}

// ------------------------------------------------------------
// Invalid opcodes
// ------------------------------------------------------------

void test_invalid_opcode() {
    Chip8 c;

    bool threw = false;

    try {
        c.execute(0xFFFF);
    }
    catch (const std::runtime_error&) {
        threw = true;
    }

    assert(threw);
}

// ------------------------------------------------------------
// Display
// ------------------------------------------------------------

static bool px(const Chip8& c, int x, int y) {
    return c.display[y * Chip8::DISPLAY_WIDTH + x] != 0;
}

void test_draw_basic_and_xor() {
    Chip8 c;
    c.V[0] = 0; c.V[1] = 0; c.I = Chip8::FONT_START;    // glyph "0": F0 90 90 90 F0
    c.execute(0xD015);

    assert(px(c,0,0) && px(c,3,0) && !px(c,4,0));       // row 0: ████....
    assert(px(c,0,1) && !px(c,1,1) && px(c,3,1));       // row 1: █..█....
    assert(c.V[0xF] == 0);

    c.execute(0xD015);                                  // draw again: erases
    for (int y = 0; y < 5; ++y) {
        for (int x = 0; x < 8; ++x) {
            assert(!px(c,x,y));
        }
    }
    assert(c.V[0xF] == 1);
}

void test_draw_wrap_start() {
    Chip8 c;
    c.V[0] = 70; c.V[1] = 40; c.I = Chip8::FONT_START;
    c.execute(0xD015);

    assert(px(c,6,8) && px(c,9,8));                      // (70,40) -> (6,8)
}

void test_draw_clip_right() {
    Chip8 c;
    c.V[0] = 60; c.V[1] = 0; c.I = Chip8::FONT_START;
    c.execute(0xD015);
    
    assert(px(c,60,0) && px(c,63,0));                    // visible part drawn
    assert(!px(c,0,0) && !px(c,3,0));                    // NOT wrapped to the left
}

void test_clear_screen() {
    Chip8 c;
    c.V[0] = 0; c.V[1] = 0; c.I = Chip8::FONT_START;
    c.execute(0xD015);
    c.execute(0x00E0);
    
    for (int i = 0; i < 64 * 32; ++i) {
        assert(c.display[i] == 0);
    }
}

void test_EX9E_skips_when_pressed() {
    Chip8 c;
    c.V[2] = 5;
    c.keys[5] = true;
    uint16_t before = c.pc;
    c.execute(0xE29E);

    assert(c.pc == before + 2);
}

void test_EX9E_does_not_skips_when_not_pressed() {
    Chip8 c;
    c.V[2] = 5;
    c.keys[5] = false;
    uint16_t before = c.pc;
    c.execute(0xE29E);

    assert(c.pc == before);
}

void test_EXA1_skips_when_not_pressed() {
    Chip8 c;
    c.V[2] = 5;
    c.keys[5] = false;
    uint16_t before = c.pc;
    c.execute(0xE2A1);

    assert(c.pc == before + 2);
}

void test_EXA1_does_not_skip_when_pressed() {
    Chip8 c;
    c.V[2] = 5;
    c.keys[5] = true;
    uint16_t before = c.pc;
    c.execute(0xE2A1);

    assert(c.pc == before);
}

void test_EX9E_masks_key_index() {
    Chip8 c;
    c.V[2] = 0x1F;
    c.keys[0xF] = true;
    uint16_t before = c.pc;
    c.execute(0xE29E);

    assert(c.pc == before + 2);
}

void test_invalid_EX2FF() {
    Chip8 c;

    try {
        c.execute(0xE2FF);

        // If we reach here, no exception was thrown → test fails
        assert(false);
    }
    catch (const std::runtime_error&) {
        // Expected: invalid opcode should throw
    }
}

// ------------------------------------------------------------
// Timers
// ------------------------------------------------------------

void test_FX07_reads_delay_timer() {
    Chip8 c;
    c.delay_timer = 42;
    c.execute(0xF207);

    assert(c.V[2] == 42);
}

void test_FX15_sets_delay_timer() {
    Chip8 c;
    c.V[2] = 42;
    c.execute(0xF215);

    assert(c.delay_timer == 42);
}

void test_FX18_sets_sound_timer() {
    Chip8 c;
    c.V[2] = 42;
    c.execute(0xF218);

    assert(c.sound_timer == 42);
}

void test_updateTimers_decrements() {
    Chip8 c;
    c.delay_timer = 5;
    c.sound_timer = 3;
    c.updateTimers();

    assert(c.delay_timer == 4);
    assert(c.sound_timer == 2);
}

void test_updateTimers_stops_at_zero() {
    Chip8 c;
    c.delay_timer = 0;
    c.sound_timer = 0;
    c.updateTimers();
    
    assert(c.delay_timer == 0);   // not 255
    assert(c.sound_timer == 0);
}

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------

int main() {

    test_return();
    test_return_stack_underflow();

    test_jump();

    test_call_and_return();
    test_call_stack_overflow();

    test_skip_if_equal();
    test_skip_if_equal_not_taken();

    test_skip_if_not_equal();
    test_skip_if_not_equal_not_taken();

    test_skip_if_registers_equal();
    test_skip_if_registers_not_equal();

    test_set_register();

    test_add_value_to_register();
    test_add_value_wraps();

    test_register_move();
    test_or();
    test_and();
    test_xor();

    test_add_registers_with_carry();
    test_add_registers_without_carry();
    test_8XY4_flag_wins_when_X_is_VF();

    test_subtract_registers_without_borrow();
    test_subtract_registers_with_borrow();

    test_shift_right();
    test_shift_right_zero_bit();

    test_reverse_subtract_without_borrow();
    test_reverse_subtract_with_borrow();

    test_shift_left();
    test_shift_left_zero_bit();

    test_skip_if_registers_not_equal_9();
    test_skip_if_registers_equal_9();

    test_set_I();

    test_random_and();

    test_add_VX_to_I();
    test_font_address();

    test_bcd();
    test_bcd_with_leading_zero();

    test_store_registers();
    test_load_registers();

    test_invalid_opcode();

    test_draw_basic_and_xor();
    test_draw_wrap_start();
    test_draw_clip_right();
    test_clear_screen();

    test_EX9E_skips_when_pressed();
    test_EX9E_does_not_skips_when_not_pressed();
    test_EXA1_skips_when_not_pressed();
    test_EXA1_does_not_skip_when_pressed();
    test_EX9E_masks_key_index();
    test_invalid_EX2FF();

    test_FX07_reads_delay_timer();
    test_FX15_sets_delay_timer();
    test_FX18_sets_sound_timer();
    test_updateTimers_decrements();
    test_updateTimers_stops_at_zero();

    std::cout << "All tests passed!\n";

    return 0;
}