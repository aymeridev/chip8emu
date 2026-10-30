#pragma once
#include <array>
#include <cstdint>

#include "roms.hpp"


enum class MenuAction {
    LOAD_ROM,
    INVERT,
};

constexpr std::uint8_t menu_size = roms.size() + 1;

class Menu {
public:
    void draw_menu(std::array<std::uint8_t, 1024>& screen_buffer);

    void previous();
    void next();
    MenuAction action() const;

    [[nodiscard]] std::uint8_t selected() const { return  selected_index; }
    bool dirty = true;
private:
    std::uint8_t selected_index = 0;
};

