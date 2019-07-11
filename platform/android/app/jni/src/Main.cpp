#include "SDL.h"

#include <android/log.h>
#include <jni.h>

#include "../../../../../core/Emulator.h"

int main(int argc, char* argv[]) {
    // init
    Log(nullptr, "Initialising SDL");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        Log(nullptr, "Could not init SDL. Error: %s", SDL_GetError());
        return 1;
    }
    int width, height;
    SDL_DisplayMode displayMode;
    if (SDL_GetCurrentDisplayMode(0, &displayMode) == 0) {
        width = displayMode.w;
        height = displayMode.h;
    }

    SDL_Window* window = SDL_CreateWindow("Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        Log(nullptr, "Could not create SDL window. Error: %s", SDL_GetError());
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        Log(nullptr, "Could not create SDL renderer. Error: %s", SDL_GetError());
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);
    if (texture == nullptr) {
        Log(nullptr, "Could not create SDL texture. Error: %s", SDL_GetError());
        return 1;
    }
    int pitch = 0;
    std::vector<unsigned char> placeholder(160*144*4, 0xFF);
    unsigned char* ph = placeholder.data();

    SDL_Rect viewport;
    int scale = 6;
    viewport.x = displayMode.w/2 - 160*scale/2, viewport.y = 50;
    viewport.w = 160*scale, viewport.h = 144*scale;

    Log(nullptr, "Successfully created SDL context with window size %dx%d", width, height);
    SDL_SetRenderDrawColor(renderer, 0x7F, 0x7F, 0x7F, 0xFF);

    Log(nullptr, "Starting emulator");
    Emulator emu;

    std::string filePath = "";
    emu.load(filePath);

    bool done = false;
    SDL_Event event;
    while (!done) {
        // emulation loop
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) done = true;
        }

        SDL_SetRenderDrawColor(renderer, 0x7F, 0x7F, 0x7F, 0xFF);
        SDL_RenderClear(renderer);

        pitch = 0;
        SDL_LockTexture(texture, nullptr, (void**) &ph, &pitch);
        SDL_UnlockTexture(texture);
        SDL_RenderCopy(renderer, texture, nullptr, &viewport);

        SDL_RenderPresent(renderer);

    }

    // shutdown
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    Log(nullptr, "Shutting down emulator");
    return 0;
}