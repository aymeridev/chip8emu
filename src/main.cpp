#define SDL_MAIN_USE_CALLBACKS 1

#include <imgui_impl_sdl3.h>
#include <iostream>
#include <memory>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "AppState.hpp"
#include "DebugUI.hpp"
#include "string"
#include "Chip8State.hpp"

using namespace std::string_literals;


constexpr float pixel_scale = 24.0f;



SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    auto* state = new AppState;
    *appstate = state;

    SDL_SetAppMetadata("chip8emu", "1.0", "dev.aymeri.chip8emu");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }


    if (!SDL_CreateWindowAndRenderer(
        "chip8emu", 1280, 720,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY,
        &state->window, &state->renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_SetRenderVSync(state->renderer, 1)) {
        SDL_Log("VSync not available: %s", SDL_GetError());
    }

    if (!state->chip8_state.load_rom("tests/1-chip8-logo.ch8"s)) {
    // if (!state->chip8_state.load_rom("tests/1-chip8-logo.ch8"s)) {
        std::cerr << "couldn't load rom";
        return SDL_APP_FAILURE;
    }
    state->chip8_state.fetch();

    state->debug_ui = std::make_unique<DebugUI>(state);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *, SDL_Event *event) {

    ImGui_ImplSDL3_ProcessEvent(event);

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {

    const auto* state = static_cast<AppState*>(appstate);

    SDL_SetRenderScale(state->renderer, 1.0f, 1.0f);
    SDL_SetRenderDrawColorFloat(state->renderer,
         state->background_color[0],
        state->background_color[1],
        state->background_color[2], 1.0f);
    SDL_RenderClear(state->renderer);

    SDL_SetRenderDrawColorFloat(state->renderer,
         state->foreground_color[0],
        state->foreground_color[1],
        state->foreground_color[2],1.0f);
    for (int y = 0; y < screen_height; y++) {
        for (int x = 0; x < screen_width; ++x) {
            if (state->chip8_state.screen[y][x]) {
                const SDL_FRect rect = {
                    static_cast<float>(x) * pixel_scale, static_cast<float>(y) * pixel_scale,
                    pixel_scale, pixel_scale
                };
                SDL_RenderFillRect(state->renderer, &rect);
            }
        }
    }


    if (state->debug_ui) state->debug_ui->update();
    SDL_RenderPresent(state->renderer);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult) {
    delete static_cast<AppState*>(appstate);
}