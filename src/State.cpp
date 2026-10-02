#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <iomanip>

#include "State.hpp"


bool State::load_rom(const std::string &file_path) {
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


void State::fetch() {
    const std::uint8_t left = memory[pc];
    const std::uint8_t right = memory[pc + 1];
    const std::uint16_t instr = (left << 8) | right;

    decode(instr);
    pc += 2;
}


void State::decode(std::uint16_t instruction) {}
