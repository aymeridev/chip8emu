#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <fstream>

inline std::vector<std::string> list_roms_from_dir(const std::string& path) {
    std::vector<std::string> roms;
    if (!std::filesystem::is_directory(path)) return roms;
    for (const auto& file : std::filesystem::directory_iterator(path)) {
        if (file.is_regular_file() && file.path().extension() == ".ch8") {
            roms.emplace_back(file.path().string());
        }
    }
    return roms;
}


inline bool load_rom_from_file(AppState* state, const std::string &file_path) {
    state->chip8_state.reset();

    std::ifstream file (file_path, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "load_rom: cannot open " << file_path << "\n";
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<std::uint8_t> bytes(size);
    file.read(reinterpret_cast<char*>(bytes.data()), size);

    if (bytes.size() > state->chip8_state.memory.size() - 0x200) {
        std::cout << "load_rom: not enough memory";
        return false;
    }

    std::ranges::copy(bytes, state->chip8_state.memory.begin() + 0x200);

    std::cout << "read " << file.gcount() << " bytes\n";

    state->rom_file_path = file_path;
    state->chip8_state.rom_loaded = true;
    return true;
}