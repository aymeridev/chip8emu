#define SDL_MAIN_USE_CALLBACKS 1

#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "string"
#include "State.hpp"
#include "ui.hpp"

using namespace std::string_literals;

static SDL_Window *window = nullptr;
static SDL_Renderer *renderer = nullptr;

constexpr float pixel_scale = 24.0f;

State state;


SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    SDL_SetAppMetadata("chip8emu", "1.0", "dev.aymeri.chip8emu");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }


    if (!SDL_CreateWindowAndRenderer(
        "chip8emu", 1280, 720,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY,
        &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (SDL_SetRenderVSync(renderer, 1)) {
        SDL_Log("VSync not available: %s", SDL_GetError());
    }

    if (!state.load_rom("tests/1-chip8-logo.ch8"s)) {
        std::cerr << "couldn't load rom";
        return SDL_APP_FAILURE;
    }
    state.fetch();

    InitUI(window, renderer);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {

    ImGui_ImplSDL3_ProcessEvent(event);

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    static bool open = true;
    if (open) ImGui::ShowDemoWindow(&open);

    ImGui::Render();

    SDL_SetRenderScale(renderer, 1.0f, 1.0f);
    SDL_SetRenderDrawColorFloat(renderer, 0.0f, 0.0f, 0.0f, 1.0f);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 127, 255, 255);
    for (int y = 0; y < screen_height; y++) {
        for (int x = 0; x < screen_width; ++x) {
            const SDL_FRect rect = {
                static_cast<float>(x) * pixel_scale, static_cast<float>(y) * pixel_scale,
                pixel_scale, pixel_scale
            };
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    // ...
    ImGuiIO& io = ImGui::GetIO();
    SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

    SDL_RenderPresent(renderer);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {}