#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <fstream>

#include "chip8_state.hpp"

inline std::vector<std::filesystem::path> list_roms_from_dir(const std::string& path) {
    std::vector<std::filesystem::path> roms;
    if (!std::filesystem::is_directory(path)) return roms;
    for (const auto& file : std::filesystem::directory_iterator(path)) {
        if (file.is_regular_file() && file.path().extension() == ".ch8") {
            roms.emplace_back(file.path());
        }
    }

    std::ranges::sort(roms.begin(), roms.end(),
        [](const auto& a, const auto& b) {
            return a.filename() < b.filename();
        });

    return roms;
}


inline bool load_rom_from_file(AppState* state, const std::string &file_path) {

    std::ifstream file (file_path, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "load_rom: cannot open " << file_path << "\n";
        return false;
    }

    const std::streamsize size = file.tellg();
    if (size < 0) return  false;
    file.seekg(0, std::ios::beg);

    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(size));
    file.read(reinterpret_cast<char*>(bytes.data()), size);

    if (state->chip8_state.load_rom(bytes)) {
        state->rom_file_path = file_path;
        return true;
    }
    return false;
}