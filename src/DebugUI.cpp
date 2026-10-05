#include <format>

#include "AppState.hpp"
#include "DebugUI.hpp"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

DebugUI::DebugUI(AppState *appstate) :
    appstate(appstate), chip8_state(appstate->chip8_state) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::GetIO().IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    window = appstate->window;
    renderer = appstate->renderer;

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
}

namespace {
    void draw_memory_viewer(const Chip8State &state) {
        bool p_memory_open = true;
        if (!ImGui::Begin("Memory", &p_memory_open)) {
            ImGui::End();
            return;
        }

        if (ImGui::BeginTable("mem", 9)) {
            for (int row = 0x200; row < 4096; row += 16) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%d", row);
                for (int i = 0; i < 8; ++i) {
                    ImGui::TableSetColumnIndex(1 + i);
                    const int idx = row + (i * 2);
                    if (idx == state.pc) ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(0, 0, 255, 255));
                    ImGui::Text("%04X", (state.memory[idx] << 8) | state.memory[idx + 1]);
                }
            }
            ImGui::EndTable();
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

        ImGui::End();
    }
}

void DebugUI::update() {
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();



    bool p_main_open = true;
    // ImGui::ShowDemoWindow(&p_main_open);
    if (!ImGui::Begin("CHIP-8 Debug", &p_main_open)) {
        ImGui::End();
        return;
    }

    ImGui::Text("current rom: %s", chip8_state.rom_file_path.c_str());
    ImGui::Text("program counter: %d", chip8_state.pc);
    ImGui::Text("i: %d", chip8_state.i);
    ImGui::Spacing();
    ImGui::Text("Registers");
    for (const auto& value : chip8_state.v) {
        ImGui::Text("%d", value);
    }




    if (ImGui::Button("Next")) {
        chip8_state.fetch();
    }

    ImGui::End();


    draw_memory_viewer(chip8_state);
    draw_display_settings(appstate);


    ImGui::Render();

    const ImGuiIO& io = ImGui::GetIO();
    SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
}
