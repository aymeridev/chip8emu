#include <format>

#include "app_state.hpp"
#include "debug_ui.hpp"

#include <iostream>

#include "disassembler.hpp"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "random.hpp"
#include "utils_io.hpp"

using namespace std::string_literals;

DebugUI::DebugUI(const AppState* appstate) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::GetIO().IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    auto& window = appstate->window;
    auto& renderer = appstate->renderer;

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

}

DebugUI::~DebugUI() {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void DebugUI::show_message(const char *message) {
    m_show_message = true;
    m_message = message;
}

namespace {

    constexpr uint32_t default_window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

    void draw_memory_viewer(Chip8State &state, const ImVec2 pos, const ImVec2 size) {
        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(size);
        if (!ImGui::Begin("Hex Viewer", nullptr, default_window_flags)) {
            ImGui::End();
            return;
        }

        if (ImGui::BeginTable("mem", 9)) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("INTERNAL");
            ImGui::PushStyleColor(ImGuiCol_TextLink, ImVec4 { 1.0f, 1.0f, 1.0f, 1.0f });
            for (int row = 0; row < memory_size; row += 16) {
                if (row == memory_data_start) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Dummy(ImVec2 { 0.0f, 32.0f });
                    ImGui::Text("CART DATA START");
                }
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%d", row);
                for (int i = 0; i < 8; ++i) {
                    ImGui::TableSetColumnIndex(1 + i);
                    const uint16_t idx = row + (i * 2);
                    const int instr = (state.memory[idx] << 8) | state.memory[idx + 1];
                    if (idx == state.pc) ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(0, 0, 255, 255));

                    ImGui::PushID(idx);
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
                        ImGui::SetTooltip("%s", disassemble(instr).c_str());
                    }
                    if (instr == 0x0) ImGui::BeginDisabled();
                    if (ImGui::TextLink(std::format("{:04X}", instr).c_str())) {
                        state.pc = idx;
                    }
                    if (instr == 0x0) ImGui::EndDisabled();
                    ImGui::PopID();
                }
            }
            ImGui::PopStyleColor();
            ImGui::EndTable();
        }

        ImGui::End();
    }

    constexpr std::array<u_int8_t, 16> draw_order = {
        0x1, 0x2, 0x3, 0xC,
        0x4, 0x5, 0x6, 0xD,
        0x7, 0x8, 0x9, 0xE,
        0xA, 0x0, 0xB, 0xF };

    void draw_main_settings(AppState* appstate, const ImVec2 pos, const ImVec2 size) {
        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(size);
        if (!ImGui::Begin("CHIP-8 Debug", nullptr, default_window_flags)) {
            ImGui::End();
            return;
        }

        ImGui::SeparatorText("Current Rom");




        if (!appstate->chip8_state.rom_loaded) {
            ImGui::Text("No ROM loaded.");
            ImGui::Text("You can select one with the menu.");
        } else {
            ImGui::SliderInt("Cycles Per Tick", &appstate->cycles_per_tick, 1, 1000);

            ImGui::Checkbox("Pause Execution", &appstate->pause);
            if (ImGui::Button("Step")) {
                appstate->run_frame();
                appstate->pause = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Next Frame")) {
                if (const auto err = appstate->chip8_state.cycle(); err != Chip8Error::PcOutOfBounds) {
                    appstate->debug_ui->show_message(std::format(
                        "Error Occured ({})",
                        static_cast<int>(err)).data());
                }
                appstate->pause = true;
            }
            if (ImGui::Button("Reset")) {
                load_rom_from_file(appstate, appstate->rom_file_path);
                appstate->pause = false;
            }
            ImGui::Text("Current Rom: %s", appstate->rom_file_path.c_str());
            ImGui::Text("Program Counter: %d", appstate->chip8_state.pc);
            ImGui::Text("Stack Pointer: %d", appstate->chip8_state.sp);
            ImGui::Text("Sound Timer: %d", appstate->chip8_state.sound_timer);
            ImGui::Text("i Register: %d", appstate->chip8_state.i_reg);
            ImGui::Text("On the Memory Viewer tab, you can also\nclick a word to jump to it.");

            ImGui::SeparatorText("Keyboard");

            int idx = 0;
            for (const auto key_order: draw_order) {
                const auto key = AppState::keys[key_order];
                if (!appstate->chip8_state.keys[key_order]) ImGui::BeginDisabled();

                ImGui::PushID(idx);
                ImGui::Text("%X (%s)", key_order, SDL_GetKeyName(key));
                ImGui::PopID();
                if (!appstate->chip8_state.keys[key_order]) ImGui::EndDisabled();
                idx++;
                if (idx % 4 != 0) ImGui::SameLine();
            }
        }

        const std::string& current_path = appstate->rom_file_path;
        if (ImGui::BeginCombo("roms list", current_path.empty() ? "no rom loaded" : current_path.data())) {
            ImGui::Selectable("Demo Roms", false, ImGuiSelectableFlags_Highlight | ImGuiSelectableFlags_Disabled);
            bool games_section = false;
            for (const auto&[path, name, is_game] : appstate->roms) {
                if (is_game && !games_section) {
                    ImGui::Spacing();
                    ImGui::Selectable("Game Roms", false, ImGuiSelectableFlags_Highlight | ImGuiSelectableFlags_Disabled);
                    games_section = true;
                }
                if (ImGui::Selectable(name.data(), path == current_path)) {
                    load_rom_from_file(appstate, path);
                }
            }
            ImGui::EndCombo();
        }

        ImGui::End();
    }

    void draw_disassembler(AppState* appstate, const ImVec2 pos, const ImVec2 size) {
        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(size);
        const auto chip = appstate->chip8_state;


        if (!ImGui::Begin("Disassembler", nullptr, default_window_flags)) {
            ImGui::End();
            return;
        }

        for (int i = memory_data_start; i < memory_size; i += 2) {
            const std::uint16_t instr = (chip.memory[i] << 8) | chip.memory[i + 1];

            if (instr == 0x0000) continue;
            ImGui::PushID(i);
            if (ImGui::RadioButton("##", std::ranges::find(appstate->breakpoints, i) != appstate->breakpoints.end())) {
                // appstate->breakpoints.
            }
            ImGui::PopID();
            ImGui::SameLine();
            ImGui::Text("%s", disassemble(instr).c_str());
        }

        ImGui::End();
    }

    void draw_registers(AppState* appstate, const ImVec2 pos, const ImVec2 size) {
        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(size);
        if (!ImGui::Begin("Registers", nullptr, default_window_flags)) {
            ImGui::End();
            return;
        }

        ImGui::Text("Click on a register to change it name");
        ImGui::BeginTable("registers_table", 2);
        for (std::size_t i = 0; i < appstate->chip8_state.v.size(); i++) {
            ImGui::TableNextColumn();
            ImGui::Text("0x%X", static_cast<int>(i));
            ImGui::SameLine();
            ImGui::PushID(static_cast<int>(i));
            ImGui::InputText("##register_name", appstate->register_names[i].data(), sizeof(appstate->register_names[i]), ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::PopID();
            ImGui::SameLine();
            ImGui::Text("%d", appstate->chip8_state.v[i]);

        }
        ImGui::EndTable();

        ImGui::End();
    }


    void draw_emulator_settings(AppState *appstate, const ImVec2 pos, const ImVec2 size) {

        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(size);

        if (!ImGui::Begin("Emulator Settings", nullptr, default_window_flags)) {
            ImGui::End();
            return;
        }

        ImGui::SeparatorText("Display");
        ImGui::Text("Background Color");

        if (ImGui::ColorEdit3("##background", appstate->background_color.data())) {
            appstate->chip8_state.screen_dirty = true;
        }
        ImGui::Text("Foreground Color");
        if (ImGui::ColorEdit3("##foreground", appstate->foreground_color.data())) {
            appstate->chip8_state.screen_dirty = true;
        }

        if (ImGui::Button("Randomize colors")) {
            appstate->background_color = { random_float() * 0.5f, random_float() * 0.5f, random_float() * 0.5f };
            appstate->foreground_color = { random_float(), random_float(), random_float() };
            appstate->chip8_state.screen_dirty = true;
        }
        ImGui::SeparatorText("Audio");
        if (ImGui::Checkbox("Enable Audio", &appstate->enable_audio)) {
            appstate->enable_audio = !appstate->enable_audio;
        }

        if (!appstate->enable_audio) ImGui::BeginDisabled();

        ImGui::Text("Frequency");
        ImGui::SliderFloat("##frequency", &appstate->frequency, 0.0f, 1000.0f);
        ImGui::Text("Volume");
        ImGui::SliderFloat("##volume", &appstate->volume, 0.0f, 1.0f);

        if (!appstate->enable_audio) ImGui::EndDisabled();

        ImGui::End();
    }

    // TODO : find a way to not use DrawList
    void draw_screen(AppState* appstate, const ImVec2 pos, const ImVec2 size) {
        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(size);
        ImGui::Begin("Screen", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        const ImVec2 p = ImGui::GetCursorScreenPos();
        draw_list->AddCallback(ImGui::GetPlatformIO().DrawCallback_SetSamplerNearest);
        draw_list->AddImage(ImTextureRef(
            reinterpret_cast<ImTextureID>(appstate->texture)
        ), ImVec2(p.x, p.y),
        ImVec2(p.x + size.x, p.y + size.y));
        ImGui::End();
    }
}

void DebugUI::update(AppState *appstate) {
    auto chip8_state = appstate->chip8_state;
    const auto& renderer = appstate->renderer;

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    const ImGuiViewport* vp = ImGui::GetMainViewport();
    const ImVec2 pos = vp->WorkPos;
    const ImVec2 size = vp->WorkSize;

    draw_disassembler(appstate, pos, ImVec2(360.0f, size.y - 240.0f));
    draw_registers(appstate, ImVec2(pos.x, size.y - 240.0f), ImVec2(360.0f, 240.0f));
    draw_memory_viewer(chip8_state, ImVec2(size.x - 360.0f, pos.y), ImVec2(360.0f, size.y));

    const float screen_window_width = size.x - 720;
    const float screen_window_height = screen_window_width / 2.0f;
    draw_screen(appstate, ImVec2(360.0f, 0.0f),
        ImVec2(screen_window_width, screen_window_height));

    draw_main_settings(appstate,
        ImVec2(360.0f, screen_window_height),
        ImVec2(screen_window_width - 200.0f, size.y - screen_window_height));

    draw_emulator_settings(appstate,
        ImVec2(360.0f + (screen_window_width - 200.0f), screen_window_height),
        ImVec2(200.0f, size.y - screen_window_height));
    if (m_show_message) ImGui::OpenPopup("Message");
    if (!m_message.empty()) {
        const ImVec2 center = vp->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("Message", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("%s", m_message.c_str());
            if (ImGui::Button("OK")) {
                m_message.clear();
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    ImGui::Render();

    const ImGuiIO& io = ImGui::GetIO();
    SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
}
