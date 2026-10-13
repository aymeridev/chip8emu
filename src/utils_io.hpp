#pragma once
#include <string>
#include <vector>
#include <filesystem>

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