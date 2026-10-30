#include "menu.hpp"
#include "main.h"
#include "roms.hpp"
#include "font5x7.hpp"
#include <string_view>


void draw_text(std::array<std::uint8_t, 1024>& buffer, int page, int column, std::string_view text) {
    for (int ch = 0; ch < text.size(); ++ch) {
        for (int x = 0; x < 5; ++x) {
            const std::uint8_t buf_col = (column + ch) * 6 + x;
            if (buf_col >= 128) return;
            buffer[128 * page + buf_col] = font[static_cast<std::uint8_t>(text[ch]) * 5 + x];
        }
    }
}

MenuAction Menu::action() const {
    if (selected_index >= roms.size()) return MenuAction::INVERT;
    return MenuAction::LOAD_ROM;
}

void Menu::next() {
    selected_index = (selected_index + 1) % menu_size;
    dirty = true;
}

void Menu::previous() {
    selected_index = (selected_index + menu_size - 1) % menu_size;
    dirty = true;
}

void Menu::draw_menu(std::array<std::uint8_t, 1024>& screen_buffer, TIM_HandleTypeDef * htim3) {
    screen_buffer.fill(0);
    draw_text(screen_buffer, 0, 0, "=====> chip8emu <====");
    for (int i = 0; i < roms.size(); ++i) {
        draw_text(screen_buffer, i + 1, 1, roms[i].name);
    }
        draw_text(screen_buffer, roms.size() + 1, 1, "[Opt] Invert");
    for (int i = 128 * (selected_index + 1); i < 128 * (selected_index + 2); ++i) {
        screen_buffer[i] = ~screen_buffer[i];
    }
}