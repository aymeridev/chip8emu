#pragma once
#include "main.h"
#include <array>
#include <cstdint>


class Menu {
public:
    void draw_menu(std::array<std::uint8_t, 1024>& screen_buffer, TIM_HandleTypeDef * htim3);

    /// return true if reached the end of the list
    bool previous();
    /// return true if reached the start of the list
    bool next();
    [[nodiscard]] std::uint8_t selected() const { return  selected_index; }
    bool dirty = true;
private:
    std::uint8_t selected_index = 0;
};

