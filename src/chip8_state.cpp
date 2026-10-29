#include <algorithm>

#include "chip8_state.hpp"

#include "logo.hpp"
#include "random.hpp"


constexpr std::array<std::uint8_t, 80> font = {
    /* 0 */ 0xF0, 0x90, 0x90, 0x90, 0xF0,
    /* 1 */ 0x20, 0x60, 0x20, 0x20, 0x70,
    /* 2 */ 0xF0, 0x10, 0xF0, 0x80, 0xF0,
    /* 3 */ 0xF0, 0x10, 0xF0, 0x10, 0xF0,
    /* 4 */ 0x90, 0x90, 0xF0, 0x10, 0x10,
    /* 5 */ 0xF0, 0x80, 0xF0, 0x10, 0xF0,
    /* 6 */ 0xF0, 0x80, 0xF0, 0x90, 0xF0,
    /* 7 */ 0xF0, 0x10, 0x20, 0x40, 0x40,
    /* 8 */ 0xF0, 0x90, 0xF0, 0x90, 0xF0,
    /* 9 */ 0xF0, 0x90, 0xF0, 0x10, 0xF0,
    /* A */ 0xF0, 0x90, 0xF0, 0x90, 0x90,
    /* B */ 0xE0, 0x90, 0xE0, 0x90, 0xE0,
    /* C */ 0xF0, 0x80, 0x80, 0x80, 0xF0,
    /* D */ 0xE0, 0x90, 0x90, 0x90, 0xE0,
    /* E */ 0xF0, 0x80, 0xF0, 0x80, 0xF0,
    /* F */ 0xF0, 0x80, 0xF0, 0x80, 0x80,
};

Chip8State::Chip8State() {
    std::ranges::copy(font, memory.begin() + font_start);

    // logo
    for (std::uint8_t y = 0; y < screen_height; ++y) {
        for (std::uint8_t x = 0; x < screen_width; ++x) {
            if (get_pixel(x, y)) screen[y] |= (std::uint64_t { 1 } << x);
        }
    }
}

void Chip8State::tick_timers() {
    if (delay_timer > 0) delay_timer--;
    if (sound_timer >0) sound_timer--;
    waiting_for_vblank = false;
}

void Chip8State::reset() {
    rom_loaded = false;
    memory.fill(0);
    screen.fill(0);

    // logo
    for (std::uint8_t y = 0; y < screen_height; ++y) {
        for (std::uint8_t x = 0; x < screen_width; ++x) {
            if (get_pixel(x, y)) screen[y] |= (std::uint64_t { 1 } << x);
            else screen[y] |= (std::uint64_t { 0 } << x);
        }
    }

    screen_dirty = true;
    pc = memory_data_start;
    v.fill(0);
    i_reg = 0;

    delay_timer = 0;
    sound_timer = 0;

    waiting_for_vblank = false;

    keys.fill(false);
    register_listening_for_key = std::nullopt;

    stack.fill(0);
    std::ranges::copy(font, memory.begin() + font_start);
    sp = 0;
}



bool Chip8State::load_rom(std::span<const std::uint8_t> rom_data) {
    reset();
    screen.fill(0);
    if (rom_data.size() > memory_size - memory_data_start) {
        return false;
    }
    std::ranges::copy(rom_data, memory.begin() + memory_data_start);
    rom_loaded = true;
    return true;
}

Chip8Error Chip8State::cycle() {
    if (pc >= memory_size - 1) return Chip8Error::PcOutOfBounds;

    // see Fx0A : stop execution until the target key is pressed
    if (register_listening_for_key.has_value()) return Chip8Error::None;

    if (waiting_for_vblank) return Chip8Error::None;
    return decode_and_execute(fetch());
}

void Chip8State::update_key(const std::uint8_t index, const bool enabled) {
    if (index >= 16) return;

    // react when pressed then released
    if (register_listening_for_key.has_value() && keys[index] && !enabled) {
        v[register_listening_for_key.value()] = index;
        register_listening_for_key = std::nullopt;
    }
    keys[index] = enabled;
}


std::uint16_t Chip8State::fetch() {
    const std::uint8_t left = memory[pc];
    const std::uint8_t right = memory[pc + 1];
    const auto instr = static_cast<uint16_t>((left << 8) | right);

    pc += 2;

    return instr;
}

Chip8Error Chip8State::decode_and_execute(uint16_t instruction) {

    const std::uint16_t nnn = instruction & 0x0FFF;
    const auto nn = static_cast<std::uint8_t>(instruction & 0x00FF);
    const std::uint8_t n = instruction & 0x000F;
    const std::uint8_t x = (instruction >> 8) & 0x0F;
    const std::uint8_t y = (instruction >> 4) & 0x0F;

    switch (instruction >> 12) { // group of instruction
        case 0:
            if (instruction == 0x00E0) { // CLS
                screen.fill(0);
                screen_dirty = true;
            } else if (instruction == 0x00EE) { // RET
                if (sp == 0) return Chip8Error::StackUnderflow;
                sp--;
                pc = stack[sp];
            }
            break;
        case 1: // JUMP
            pc = nnn;
            break;
        case 2:
            if (sp >= stack.size()) return Chip8Error::StackOverflow;
            stack[sp] = pc;
            sp++;
            pc = nnn;
            break;
        case 3:
            if (pc < 0xFFF && v[x] == nn) {
                pc += 2;
            }
            break;
        case 4:
            if (pc < 0xFFF && v[x] != nn) {
                pc += 2;
            }
            break;
        case 5:
            if (pc < 0xFFF && n == 0 && v[x] == v[y]) pc += 2;
            /* XO-CHIP */ if (n == 2) break;
            /* XO-CHIP */ if (n == 3) break;
            break;
        case 6:
            v[x] = nn;
            break;
        case 7:
            v[x] += nn;
            break;
        case 8:
            return run_math(n, x, y);
        case 9:
            if (n == 0 && v[x] != v[y]) {
                pc += 2;
            }
            break;
        case 0xA:
            i_reg = nnn;
            break;
        case 0xB:
            pc = (nnn + v[0]) & 0x0FFF;
            break;
        case 0xC:
            v[x] = static_cast<uint8_t>(xorshift32()) & nn;
            break;
        case 0xD: { // display
            screen_dirty = true;

            // get coordinates
            const int cx = v[x] % screen_width;
            const int cy = v[y] % screen_height;
            v[0xF] = 0;
            for (int mem_y = 0; mem_y < n; ++mem_y) {
                const std::uint8_t row = memory[(i_reg + mem_y) & 0x0FFF];

                for (int sc_x = 0; sc_x < 8; ++sc_x) {
                    const int dx = cx + sc_x;
                    const int dy = cy + mem_y;

                    if (dx >= screen_width) break;
                    if (dy >= screen_height) {
                        waiting_for_vblank = true;
                        return Chip8Error::None;
                    }

                    const bool new_pixel = (row >> (7 - sc_x)) & 1;
                    if (((screen[static_cast<std::size_t>(dy)] >> dx) & 1) && new_pixel) {
                        v[0xF] = 1;
                    }
                    if (new_pixel) screen[static_cast<std::size_t>(dy)] ^= (std::uint64_t { 1 } << dx);
                }
            }

            waiting_for_vblank = true;

            break;
        }
        case 0xE:
            switch (nn) {
                case 0x9E: // SKP Vx
                    if (keys[v[x] % 16]) pc += 2;
                    break;
                case 0xA1: // SKNP Vx
                    if (!keys[v[x] % 16]) pc += 2;
                    break;
                default:
                    return Chip8Error::UnknownOpcode;
            }
            break;
        case 0xF:
            switch (nn) {
                case 0x00:
                    if (pc < 0xFFF) {
                        i_reg = fetch();
                    }
                    break;
                case 0x07:
                    v[x] = delay_timer;
                    break;
                case 0x0A:
                    register_listening_for_key = x;
                    break;
                case 0x15:
                    delay_timer = v[x];
                    break;
                case 0x18:
                    sound_timer = v[x];
                    break;
                case 0x29: // hex vX
                    i_reg = static_cast<std::uint16_t>(font_start + (v[x] % 16) * 5);
                    break;
                case 0x33: // bcd
                    if (i_reg < memory_size    ) memory[i_reg] = static_cast<std::uint8_t>(v[x] / 100);
                    if (i_reg < memory_size - 1) memory[i_reg + 1] = static_cast<std::uint8_t>(v[x] / 10 % 10);
                    if (i_reg < memory_size - 2) memory[i_reg + 2] = v[x] % 10;
                    break;
                case 0x55: // save
                    for (std::size_t idx = 0; idx <= x; ++idx) {
                        if (i_reg + idx < memory_size) memory[i_reg + idx] = v[idx];
                    }
                    i_reg += static_cast<std::uint16_t>(x + 1);
                    break;
                case 0x65: // load
                    for (std::size_t idx = 0; idx <= x; ++idx) {
                        if (i_reg + idx < memory_size) v[idx] = memory[i_reg + idx];
                    }
                    i_reg += static_cast<std::uint16_t>(x + 1);
                    break;

                case 0x1E:
                    i_reg += v[x];
                    break;
                default:
                    return Chip8Error::UnknownOpcode;
            }
            break;
        default:
            return Chip8Error::UnknownOpcode;
    }
    return Chip8Error::None;
}


Chip8Error Chip8State::run_math(std::uint8_t n, std::uint8_t x, std::uint8_t y) {
    switch (n) {
        case 0: // assign
            v[x] = v[y];
            break;
        case 1: // or
            v[x] |= v[y];
            v[0xF] = 0;
            break;
        case 2: // and
            v[x] &= v[y];
            v[0xF] = 0;
            break;
        case 3: // xor
            v[x] ^= v[y];
            v[0xF] = 0;
            break;
        case 4: {
            // add, if overflow set vF to 1 else 0
            const std::uint16_t res = v[x] + v[y];
            v[x] = static_cast<std::uint8_t>(res);

            v[0xF] = res > 0x00FF ? 1 : 0;
            break;
        }
        case 5: {
            // subtract, if underflow set vF to 0 else 1
            const std::uint16_t res = v[x] - v[y];
            v[x] = static_cast<std::uint8_t>(res);

            v[0xF] = res > 0x00FF ? 0 : 1;
            break;
        }
        case 6: {
            // set vX to vY and shift vX one bit to the right, set vF to the bit shifted out, even if X=F! [Quirk 6]
            v[x] = v[y];
            const std::uint8_t bit = v[x] & 1;
            v[x] >>= 1;

            v[0xF] = bit;

            break;
        }
        case 7: {
            // set vX to the result of subtracting vX from vY, vF is set to 0 if an underflow happened, to 1 if not, even if X=F!
            const std::uint16_t res = v[y] - v[x];
            v[x] = static_cast<std::uint8_t>(res);
            v[0xF] = res > 0x00FF ? 0 : 1;
            break;
        }
        case 0xE: {
            // set vX to vY and shift vX one bit to the left, set vF to the bit shifted out, even if X=F!
            v[x] = v[y];
            const std::uint8_t bit = (v[x] & 0x80) >> 7;
            v[x] <<= 1;
            v[0xF] = bit;
            break;
        }
        default:
            return Chip8Error::UnknownOpcode;
            break;
    }
    return Chip8Error::None;
}
