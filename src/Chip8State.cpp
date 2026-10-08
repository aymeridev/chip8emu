#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>

#include "Chip8State.hpp"


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

void Chip8State::reset() {
    memory.fill(0);
    clear_screen();
    pc = 0x200;
    v.fill(0);
    i_reg = 0;

    delay_timer = 0;
    sound_timer = 0;

    stack.fill(0);
    std::ranges::copy(font, memory.begin() + 0x50);
    sp = 0;
}

bool Chip8State::load_rom(const std::string &file_path) {
    reset();

    std::ifstream file (file_path, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "load_rom: cannot open " << file_path << "\n";
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<std::uint8_t> bytes(size);
    file.read(reinterpret_cast<char*>(bytes.data()), size);

    if (bytes.size() > memory.size() - 0x200) {
        std::cout << "load_rom: not enough memory";
        return false;
    }

    std::ranges::copy(bytes, memory.begin() + 0x200);

    std::cout << "read " << file.gcount() << " bytes\n";

    rom_file_path = file_path;
    return true;
}

bool Chip8State::load_rom(std::span<uint8_t> &rom_data) {
    reset();
    if (rom_data.size() > 4096 - 0x200) {
        std::cerr << "load_rom: not enough memory";
        return false;
    }
    std::ranges::copy(rom_data, memory.begin() + 0x200);
    return true;
}



std::uint16_t Chip8State::fetch() {
    const std::uint8_t left = memory[pc];
    const std::uint8_t right = memory[pc + 1];
    const std::uint16_t instr = (left << 8) | right;

    pc += 2;

    return instr;
}

bool Chip8State::cycle() {
    if (pc >= 4094) return false;
    const auto instr = fetch();
    decode_and_execute(instr);
    return pc >= 4094;
}


void Chip8State::run_math(std::uint8_t n, std::uint8_t x, std::uint8_t y) {
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
            v[x] = res;

            v[0xF] = res > 0x00FF ? 1 : 0;
            break;
        }
        case 5: {
            // subtract, if underflow set vF to 0 else 1
            const std::uint16_t res = v[x] - v[y];
            v[x] = res;

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
            v[x] = res;
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
            std::cerr << "unknown instruction " << n << "\n";
            break;
    }
}

void Chip8State::decode_and_execute(std::uint16_t instruction) {

    const std::uint16_t nnn = instruction & 0x0FFF;
    const std::uint16_t nn = instruction & 0x00FF;
    const std::uint8_t n = instruction & 0x000F;
    const std::uint8_t x = (instruction >> 8) & 0x0F;
    const std::uint8_t y = (instruction >> 4) & 0x0F;

    // std::cout << std::setw(4) << std::hex << std::uppercase << std::setfill('0') << instruction << "\n";
    // std::cout << std::setw(4) << std::hex << std::uppercase << std::setfill('0') << "nnn: " << nnn << "\n";
    // std::cout << std::setw(4) << std::hex << std::uppercase << std::setfill('0') << "nn : " << nn << "\n";
    // std::cout << std::setw(4) << std::hex << std::uppercase << std::setfill('0') << "n  : " << +n << "\n";
    // std::cout << std::setw(4) << std::hex << std::uppercase << std::setfill('0') << "x  : " << +x << "\n";

    // std::cout << std::setw(4) << std::hex << std::uppercase << std::setfill('0') << nnn << "\n";

    switch (instruction >> 12) { // group of instruction
        case 0:
            if (instruction == 0x00E0) { // CLS
                clear_screen();
            } else if (instruction == 0x00EE) { // RET
                sp--;
                pc = stack[sp];
            }
            break;
        case 1: // JUMP
            pc = nnn;
            break;
        case 2:
            stack[sp] = pc;
            sp++;
            pc = nnn;
            break;
        case 3:
            if (v[x] == nn) {
                pc += 2;
            }
            break;
        case 4:
            if (v[x] != nn) {
                pc += 2;
            }
            break;
        case 5:
            if (n == 0 && v[x] == v[y]) pc += 2;
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
            run_math(n, x, y);
            break;
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
            v[x] = std::uniform_int_distribution<>(0, 255)(rng) & nn;
            break;
        case 0xD: { // display

            // get coordinates
            const int cx = v[x] % screen_width;
            const int cy = v[y] % screen_height;
            v[0xF] = 0;
            for (int mem_y = 0; mem_y < n; ++mem_y) {
                const std::uint8_t row = memory[i_reg + mem_y];

                for (int sc_x = 0; sc_x < 8; ++sc_x) {
                    const int dx = cx + sc_x;
                    const int dy = cy + mem_y;

                    if (dx >= screen_width) break;
                    if (dy >= screen_height) return;

                    const bool new_pixel = (row >> (7 - sc_x)) & 1;
                    if (screen[dy][dx] && !new_pixel) {
                        v[0xF] = 1;
                    }
                    screen[dy][dx] = screen[dy][dx] ^ new_pixel;
                }
            }

            break;
        }
        // case 0xE:
        //     break;
        case 0xF:
            switch (nn) {
                case 0x00:
                    i_reg = fetch();
                    break;
                case 0x07:
                    v[x] = delay_timer;
                    break;
                case 0x15:
                    delay_timer = v[x];
                    break;
                case 0x18:
                    sound_timer = v[x];
                    break;
                case 0x29: // hex vX
                    i_reg = 0x50 + (v[x] % 16) * 5;
                    break;
                case 0x33: // bcd
                    if (i_reg < 4096) memory[i_reg] = static_cast<uint8_t>(v[x] / 100);
                    if (i_reg < 4095) memory[i_reg + 1] = static_cast<uint8_t>(std::floor(v[x] / 10 % 10));
                    if (i_reg < 4094) memory[i_reg + 2] = v[x] % 10;
                    break;
                case 0x55: // save
                    for (std::size_t idx = 0; idx <= x; ++idx) {
                        if (i_reg + idx < 4096) memory[i_reg + idx] = v[idx];
                    }
                    i_reg += x + 1;
                    break;
                case 0x65: // load
                    for (std::size_t idx = 0; idx <= x; ++idx) {
                        if (i_reg + idx < 4096) v[idx] = memory[i_reg + idx];
                    }
                    i_reg += x + 1;
                    break;

                case 0x1E:
                    i_reg += v[x];
                    break;
                default:
                    std::cout << "unknown 0xFx.. instruction:" << std::setw(4) << std::hex << std::uppercase << std::setfill('0') << instruction << "\n";
                    break;
            }
            break;
        default:
            std::cout << "unknown instruction:" << std::setw(4) << std::hex << std::uppercase << std::setfill('0') << instruction << "\n";
            break;
    }
}

void Chip8State::tick_timers() {
    if (delay_timer > 0) delay_timer--;
    if (sound_timer >0) sound_timer--;
}

void Chip8State::clear_screen() {
    for (auto & row_y : screen) {
        for (bool & pixel_x : row_y) {
            pixel_x = false;
        }
    }
}