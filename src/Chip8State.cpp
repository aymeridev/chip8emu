#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <cstdint>

#include "Chip8State.hpp"


bool Chip8State::load_rom(const std::string &file_path) {
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


void Chip8State::fetch() {
    const std::uint8_t left = memory[pc];
    const std::uint8_t right = memory[pc + 1];
    const std::uint16_t instr = (left << 8) | right;

    decode(instr);
    pc += 2;
}


void Chip8State::decode(std::uint16_t instruction) {

    const std::uint16_t nnn = instruction & 0x0FFF;
    const std::uint16_t nn = instruction & 0x00FF;
    const std::uint8_t n = instruction & 0x000F;
    const std::uint8_t x = (instruction >> 8) & 0x0F;
    const std::uint8_t y = (instruction >> 4) & 0x0F;

    std::cout << std::setw(4) << std::hex << std::uppercase << std::setfill('0') << instruction << "\n";
    // std::cout << std::setw(4) << std::hex << std::uppercase << std::setfill('0') << "nnn: " << nnn << "\n";
    // std::cout << std::setw(4) << std::hex << std::uppercase << std::setfill('0') << "nn : " << nn << "\n";
    // std::cout << std::setw(4) << std::hex << std::uppercase << std::setfill('0') << "n  : " << +n << "\n";
    // std::cout << std::setw(4) << std::hex << std::uppercase << std::setfill('0') << "x  : " << +x << "\n";

    // std::cout << std::setw(4) << std::hex << std::uppercase << std::setfill('0') << nnn << "\n";

    std::cout << instruction << "\n";
    switch (instruction >> 12) { // group of instruction
        case 0:
            if (instruction == 0x00E0) { // CLS
                for (auto & row_y : screen) {
                    for (bool & pixel_x : row_y) {
                        pixel_x = false;
                    }
                }
            } else if (instruction == 0x00EE) { // RET
                // TODO
            }
            break;
        case 1: // JUMP
            pc = nnn;
            break;
        // case 2:
        //     break;
        // case 3:
        //     break;
        // case 4:
        //     break;
        // case 5:
        //     break;
        case 6:
            std::cout << "update [" << x << "] with" << nn << "\n";
            v[x] = nn;
            break;
        case 7:
            v[x] += nn;
            break;
        // case 8:
        //     break;
        // case 9:
        //     break;
        case 10: // a
            i = nnn;
            break;
        // case 11: // b
        //     break;
        case 12: // c
            break;
        case 13: {
            // d (DISPLAY)

            // get coordinates
            const int cx = v[x] % screen_width;
            const int cy = v[y] % screen_height;
            v[0xF] = 0;
            std::cout << "DISPLAY " << cx << "/" << cy << '\n';
            for (int mem_y = 0; mem_y < n; ++mem_y) {
                const std::uint8_t row = memory[i + mem_y];

                for (int sc_x = 0; sc_x < 8; ++sc_x) {
                    const int dx = cx + sc_x;
                    const int dy = cy + mem_y;
                    const bool new_pixel = (row >> (7 - sc_x)) & 1;
                    if (screen[dy][dx] && !new_pixel) {
                        v[0xF] = 1;
                    }
                    screen[dy][dx] = screen[dy][dx] ^ new_pixel;
                }
            }

            break;
        }
        // case 14: // e
        //     break;
        // case 15: // f
        //     break;
        // default:
        //     throw std::runtime_error("Unknown instruction");
        //     break;
    }
}
