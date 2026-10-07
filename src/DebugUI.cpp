#include <format>

#include "AppState.hpp"
#include "DebugUI.hpp"

#include <imgui_internal.h>
#include <iostream>

#include "example_roms.hpp"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

using namespace std::string_literals;

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

    void draw_logs(const ImVec2 pos, const ImVec2 size) {

    }

    void draw_memory_viewer(const Chip8State &state, const ImVec2 pos, const ImVec2 size) {
        bool p_memory_open = true;

        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(size);
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

    void draw_main_settings(AppState* appstate, const ImVec2 pos, const ImVec2 size) {
        bool p_main_open = true;
        // ImGui::ShowDemoWindow(&p_main_open);

        constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(size);
        if (!ImGui::Begin("CHIP-8 Debug", &p_main_open, flags)) {
            ImGui::End();
            return;
        }

        ImGui::Text("current rom: %s", appstate->chip8_state.rom_file_path.c_str());
        ImGui::Text("program counter: %d", appstate->chip8_state.pc);
        ImGui::Text("i: %d", appstate->chip8_state.i);
        ImGui::Spacing();
        ImGui::Text("Registers");
        for (const auto& value : appstate->chip8_state.v) {
            ImGui::Text("%d", value);
            ImGui::SameLine();
        }



        ImGui::Checkbox("pause", &appstate->pause);

        if (!appstate->pause) ImGui::BeginDisabled();
        if (ImGui::Button("Next")) {
            appstate->chip8_state.fetch();
        }
        if (!appstate->pause) ImGui::EndDisabled();

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

    void draw_rom_switcher(AppState* appstate) {
        bool p_switch_open = true;

        if (!ImGui::Begin("Switch &ROM", &p_switch_open)) {
            ImGui::End();
            return;
        }

        ImGui::Text("test roms");
        if (ImGui::BeginListBox("rom list")) {
            for (const auto path : test_roms) {
                if (ImGui::Selectable(path.data(), ("roms/"s + path.data()) == appstate->chip8_state.rom_file_path)) {
                    appstate->chip8_state.load_rom("roms/"s + path.data());
                }
            }
            ImGui::EndListBox();
        }
        ImGui::Text("example roms");

        ImGui::End();
    }

    // TODO : find a way to not use DrawList
    void draw_screen(AppState* appstate, const ImVec2 pos, const ImVec2 size) {
        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(size);
        bool open = true;
        ImGui::Begin("Screen", &open, ImGuiWindowFlags_NoTitleBar);
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

void DebugUI::update() {
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    const ImGuiViewport* vp = ImGui::GetMainViewport();
    const ImVec2 pos = vp->WorkPos;
    const ImVec2 size = vp->WorkSize;

    draw_memory_viewer(chip8_state, ImVec2(size.x - 360.0f, pos.y), ImVec2(360.0f, size.y));
    draw_display_settings(appstate);
    draw_rom_switcher(appstate);

    const float screen_window_width = size.x - 720;
    const float screen_window_height = screen_window_width / 2.0f;
    draw_screen(appstate, ImVec2(360.0f, 0.0f),
        ImVec2(screen_window_width, screen_window_height));

    draw_main_settings(appstate,
        ImVec2(360.0f, screen_window_height),
        ImVec2(screen_window_width, 64));
    ImGui::Render();

    const ImGuiIO& io = ImGui::GetIO();
    SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
}
