#define SDL_MAIN_USE_CALLBACKS 1

#include <imgui_impl_sdl3.h>
#include <iostream>
#include <memory>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <string>

#include "AppState.hpp"
#include "Chip8State.hpp"
#include "DebugUI.hpp"
#include "utils_color.hpp"

using namespace std::string_literals;

SDL_AppResult SDL_AppInit(void **appstate, int, char *[]) {
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

    state->texture = SDL_CreateTexture(state->renderer, SDL_PIXELFORMAT_ABGR32, SDL_TEXTUREACCESS_STREAMING, 64, 32);

    // prevent blur
    if (!SDL_SetTextureScaleMode(state->texture, SDL_SCALEMODE_NEAREST)) {
        SDL_Log("Couldn't set texture scale mode: %s", SDL_GetError());
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
    state->last_tick = static_cast<float>(SDL_GetTicks());

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
    auto* state = static_cast<AppState*>(appstate);

    const auto tick_now = static_cast<float>(SDL_GetTicks());

    state->cycle_timer -= (tick_now - state->last_tick) / 1000.0f;
    state->tick_timer -= (tick_now - state->last_tick) / 1000.0f;
    state->last_tick = tick_now;

    if (state->cycle_timer <= 0.0f) {
        for (int i = 0; i < 10; ++i) {
            if (!state->chip8_state.cycle()) state->pause = true;
        }
        state->cycle_timer = cycle_total_timer;
    }

    if (state->tick_timer <= 0.0f) {
        state->chip8_state.tick_timers();
        state->tick_timer = tick_total_timer;
    }

    SDL_SetRenderScale(state->renderer, 1.0f, 1.0f);

    for (int y = 0; y < screen_height; y++) {
        for (int x = 0; x < screen_width; ++x) {
            state->pixels[y * screen_width + x] = state->chip8_state.screen[y][x] ?
            color_to_uint32(state->foreground_color) : color_to_uint32(state->background_color);
        }
    }
    SDL_UpdateTexture(state->texture, nullptr, state->pixels, screen_width * sizeof(Uint32));

    if (state->debug_ui) {
        SDL_RenderClear(state->renderer);
        state->debug_ui->update();
    } else {
        SDL_SetRenderDrawColorFloat(state->renderer,
             state->background_color[0],
            state->background_color[1],
            state->background_color[2], 1.0f);
        SDL_RenderClear(state->renderer);

        SDL_SetRenderDrawColorFloat(state->renderer,
             state->foreground_color[0],
            state->foreground_color[1],
            state->foreground_color[2],1.0f);
        SDL_RenderClear(state->renderer);
        SDL_RenderTexture(state->renderer, state->texture, nullptr, nullptr);
    }
    SDL_RenderPresent(state->renderer);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult) {
    const auto state = static_cast<AppState*>(appstate);
    SDL_DestroyTexture(state->texture);
    delete state;
}