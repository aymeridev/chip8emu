#define SDL_MAIN_USE_CALLBACKS 1

#include <imgui_impl_sdl3.h>
#include <iostream>
#include <memory>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <string>
#include <filesystem>

#include "app_state.hpp"
#include "chip8_state.hpp"
#include "debug_ui.hpp"
#include "utils_color.hpp"
#include "utils_io.hpp"

using namespace std::string_literals;

namespace {
    void show_message(const AppState* state, const char* message) {
        if (state->debug_ui) state->debug_ui->show_message(message);
        else SDL_Log("%s", message);
    }
}

SDL_AppResult SDL_AppInit(void **appstate, int, char *[]) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetAppMetadata("chip8emu", "1.0", "dev.aymeri.chip8emu");

    auto* state = new AppState;
    *appstate = state;

    SDL_AudioSpec spec { SDL_AUDIO_F32, 1, 44100 };
    state->stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
    if (!state->stream) {
        SDL_Log("Couldn't open audio device: %s", SDL_GetError());
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


    state->chip8_state.reset();

    state->debug_ui = std::make_unique<DebugUI>(state);
    state->last_tick = static_cast<float>(SDL_GetTicks());

    SDL_ResumeAudioStreamDevice(state->stream);

    std::filesystem::path base = "./";

    if (SDL_GetBasePath()) base = SDL_GetBasePath();
    auto roms_base_dir = base / "roms";
    for (const auto& path: list_roms_from_dir(roms_base_dir)) {
        state->roms.emplace_back(RomEntry { .path = path, .name = std::filesystem::path(path).filename(), .is_game = false });
    }

    for (const auto& path: list_roms_from_dir(roms_base_dir / "games")) {
        state->roms.emplace_back(RomEntry { .path = path, .name = std::filesystem::path(path).filename(), .is_game = true });
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {

    auto* state = static_cast<AppState*>(appstate);

    if (state->debug_ui) ImGui_ImplSDL3_ProcessEvent(event);
    if (event->type == SDL_EVENT_KEY_DOWN or event->type == SDL_EVENT_KEY_UP) {
        std::size_t idx = 0;
        for (const auto key: AppState::keys) {
            if (event->key.key == key) {
                state->chip8_state.update_key(idx, event->type == SDL_EVENT_KEY_DOWN);
            }
            idx++;
        }
        if (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_F1) {
            if (state->debug_ui) {
                state->debug_ui.reset();
            } else {
                state->debug_ui = std::make_unique<DebugUI>(state);
            }
        }
    }
    if (event->type == SDL_EVENT_DROP_FILE) {

        const auto file_path = event->drop.data;

        std::cout << file_path << std::endl;

        if (std::filesystem::path(file_path).extension() == ".8o") {
            if (!load_rom_from_file(state, file_path)) {
                show_message(state, "Error while loading file");
            }
        } else if(state->debug_ui) {
            show_message(state, "Invalid file extension");
        }
    }

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }


    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    auto* state = static_cast<AppState*>(appstate);

    const auto tick_now = static_cast<float>(SDL_GetTicks());

    // TODO: audio
    // const int minimum_audio = (8000 * sizeof(float)) / 2;
    // if (SDL_GetAudioStreamQueued(state->stream) < minimum_audio) {
    //     static float samples[512];
    //
    //     for (size_t i = 0; i < std::size(samples); i++) {
    //     }
    //
    //     SDL_PutAudioStreamData(state->stream, samples, sizeof (samples));
    // }

    state->cycle_timer -= (tick_now - state->last_tick) / 1000.0f;
    state->tick_timer -= (tick_now - state->last_tick) / 1000.0f;
    state->last_tick = tick_now;

    if (state->cycle_timer <= 0.0f) {
        for (int i = 0; i < state->cycles_per_tick; ++i) {
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
        state->debug_ui->update(state);
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