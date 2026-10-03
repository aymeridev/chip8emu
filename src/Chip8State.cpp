#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <iomanip>

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

    if (bytes.size() > memory.size()) {
        std::cout << "load_rom: not enough memory";
        return false;
    }

    std::ranges::copy(bytes, memory.begin());

    std::cout << "read " << file.gcount() << " bytes\n";
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

    const std::uint8_t family = (instruction >> 12);

    std::cout << std::setw(4) << std::hex << std::uppercase << std::setfill('0') << instruction << "\n";


    switch (family) {
        case 0:
            if (instruction == 0x00E0) { // CLS
                for (int y = 0; y < screen_height; y++) {
                    for (int x = 0; x < screen_width; ++x) {
                        screen[y][x] = false;
                    }
                }
            } else if (instruction == 0x00EE) { // RET
                // TODO
            }
            break;
        case 1:
            break;
        case 2:
            break;
        case 3:
            break;
        case 4:
            break;
        case 5:
            break;
        case 6:
            break;
        case 7:
            break;
        case 8:
            break;
    }
}
