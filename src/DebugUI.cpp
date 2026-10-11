#include <format>

#include "AppState.hpp"
#include "DebugUI.hpp"

#include <imgui_internal.h>
#include <iostream>

#include "Disassembler.hpp"
#include "example_roms.hpp"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "random.hpp"

using namespace std::string_literals;

DebugUI::DebugUI(const AppState* appstate) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::GetIO().IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    auto window = appstate->window;
    auto renderer = appstate->renderer;

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
}

DebugUI::~DebugUI() {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

namespace {

    constexpr uint32_t default_window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

    void draw_memory_viewer(Chip8State &state, const ImVec2 pos, const ImVec2 size) {
        bool p_memory_open = true;

        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(size);
        if (!ImGui::Begin("Hex Viewer", &p_memory_open, default_window_flags)) {
            ImGui::End();
            return;
        }

        if (ImGui::BeginTable("mem", 9)) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("INTERNAL");
            ImGui::PushStyleColor(ImGuiCol_TextLink, ImVec4 { 1.0f, 1.0f, 1.0f, 1.0f });
            for (int row = 0; row < 4096; row += 16) {
                if (row == 0x200) {
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
        bool p_main_open = true;
        ImGui::ShowDemoWindow(&p_main_open);

        constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(size);
        if (!ImGui::Begin("CHIP-8 Debug", &p_main_open, flags)) {
            ImGui::End();
            return;
        }

        if (ImGui::CollapsingHeader("Debugger")) {
            ImGui::SliderInt("Cycles Per Tick", &appstate->cycles_per_tick, 1, 1000);

            ImGui::Checkbox("Pause Execution", &appstate->pause);
            ImGui::SameLine();
            if (ImGui::Button(">> Next Frame")) {
                appstate->pause = true;
                appstate->chip8_state.cycle();
            }
            ImGui::Text("On the Memory Viewer tab, you can also click a word to jump to it.");
        }

        const std::string& current_path = appstate->chip8_state.rom_file_path;
        if (ImGui::BeginCombo("roms list", current_path.empty() ? "no rom loaded" : current_path.data())) {
            ImGui::Selectable("Demo Roms", false, ImGuiSelectableFlags_Highlight | ImGuiSelectableFlags_Disabled);
            for (const auto path : test_roms) {
                if (ImGui::Selectable(path.data(), ("roms/"s + path.data()) == current_path)) {
                    appstate->chip8_state.load_rom("roms/"s + path.data());
                }
            }
            ImGui::Spacing();
            ImGui::Selectable("Game Roms", false, ImGuiSelectableFlags_Highlight | ImGuiSelectableFlags_Disabled);
            for (const auto path : example_roms) {
                if (ImGui::Selectable(path.data(), ("roms/games/"s + path.data()) == appstate->chip8_state.rom_file_path)) {
                    appstate->chip8_state.load_rom("roms/games/"s + path.data());
                }
            }

            ImGui::EndCombo();
        }


        if (appstate->chip8_state.rom_loaded) {
            ImGui::Text("current rom: %s", appstate->chip8_state.rom_file_path.c_str());
            ImGui::Text("program counter: %d", appstate->chip8_state.pc);
            ImGui::Text("i: %d", appstate->chip8_state.i_reg);
            ImGui::Spacing();
            ImGui::Text("Registers");
            for (std::size_t i = 0; i < appstate->chip8_state.v.size(); ++i) {
                ImGui::Text("0x%X", static_cast<int>(i));
                ImGui::SameLine();
                ImGui::Text("%d", appstate->chip8_state.v[i]);
                // ImGui::SameLine();
                // // std::string name = "test";
                // // ImGui::InputText("test", name.data(), sizeof(name), ImGuiInputTextFlags_EnterReturnsTrue);
            }
            ImGui::Spacing();
            ImGui::Text("Keyboard");

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
        } else {
            ImGui::Text("No ROM is loaded.");
            ImGui::Text("You can select one with the menu.");
        }

        ImGui::End();
    }

    void draw_disassembler(AppState* appstate, const ImVec2 pos, const ImVec2 size) {
        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(size);
        auto chip = appstate->chip8_state;

        bool p_main_open = true;

        if (!ImGui::Begin("Disassembler", &p_main_open)) {
            ImGui::End();
            return;
        }

        for (int i = 0x200; i < 0x0FFF; i += 2) {
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


    void draw_display_settings(AppState *appstate) {
        bool p_display_open = true;

        if (!ImGui::Begin("Display", &p_display_open)) {
            ImGui::End();
            return;
        }

        ImGui::ColorEdit3("background", appstate->background_color.data());
        ImGui::ColorEdit3("foreground", appstate->foreground_color.data());

        if (ImGui::Button("Randomize colors")) {
            appstate->background_color = { random_float() * 0.5f, random_float() * 0.5f, random_float() * 0.5f };
            appstate->foreground_color = { random_float(), random_float(), random_float() };
        }

        ImGui::End();
    }

    // TODO : find a way to not use DrawList
    void draw_screen(AppState* appstate, const ImVec2 pos, const ImVec2 size) {
        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(size);
        bool open = true;
        ImGui::Begin("Screen", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
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
    const auto renderer = appstate->renderer;

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    const ImGuiViewport* vp = ImGui::GetMainViewport();
    const ImVec2 pos = vp->WorkPos;
    const ImVec2 size = vp->WorkSize;

    draw_disassembler(appstate, pos, ImVec2(360.0f, size.y));
    draw_memory_viewer(chip8_state, ImVec2(size.x - 360.0f, pos.y), ImVec2(360.0f, size.y));
    draw_display_settings(appstate);

    const float screen_window_width = size.x - 720;
    const float screen_window_height = screen_window_width / 2.0f;
    draw_screen(appstate, ImVec2(360.0f, 0.0f),
        ImVec2(screen_window_width, screen_window_height));

    draw_main_settings(appstate,
        ImVec2(360.0f, screen_window_height),
        ImVec2(screen_window_width, size.y - screen_window_height));
    ImGui::Render();

    const ImGuiIO& io = ImGui::GetIO();
    SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
}
