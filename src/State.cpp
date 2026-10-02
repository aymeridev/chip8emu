#include <iostream>
#include <fstream>
#include <string>

#include "State.hpp"

bool State::load_rom(const std::string &file_path) {
    std::ifstream file (file_path, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "cannot open " << file_path << "\n";
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<std::uint8_t> bytes(size);
    file.read(reinterpret_cast<char*>(bytes.data()), size);

    std::cout << "read " << file.gcount() << " bytes\n";
    return true;
}
