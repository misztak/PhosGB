#include <emscripten.h>

#include <SDL.h>

#include <chrono>
#include <thread>

#include "../../core/Emulator.h"

//constexpr double frameTimeMicro = (1.0 / 60) * 1e6;
constexpr int WEB_SCALE = 4;

Emulator emu;

struct context {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_AudioDeviceID audio;
};

void render(SDL_Renderer* renderer, SDL_Texture* texture) {
    SDL_RenderClear(renderer);

    int pitch = 0;
    unsigned char* displayPtr;
    SDL_LockTexture(texture, nullptr, (void**) &displayPtr, &pitch);
    memcpy(displayPtr, emu.getDisplayState(), 160 * 144 * 4);
    SDL_UnlockTexture(texture);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);

    SDL_RenderPresent(renderer);
}

void handleJoypadInput(SDL_Event& event) {
    u8 key;
    switch (event.key.keysym.sym) {
        case SDLK_RETURN: key = 7; break;   // Start
        case SDLK_SPACE: key = 6; break;    // Select
        case SDLK_s: key = 5; break;        // B
        case SDLK_a: key = 4; break;        // A
        case SDLK_DOWN: key = 3; break;
        case SDLK_UP: key = 2; break;
        case SDLK_LEFT: key = 1; break;
        case SDLK_RIGHT: key = 0; break;
        default: return;
    }

    if (event.type == SDL_KEYDOWN) {
        emu.handleInputDown(key);
    } else if (event.type == SDL_KEYUP) {
        emu.handleInputUp(key);
    }
    Log(I, "Yeet\n");
}

extern "C" void emulationLoop(void* arg) {
    Log(I, "Starting emulation loop\n");
    // get context
    struct context* ctx = (struct context*) arg;

    int ticks = 0;
    bool shutdown = false;
    SDL_Event event;
    while (!shutdown) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                shutdown = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(ctx->window))
                shutdown = true;
            handleJoypadInput(event);
        }
        if (shutdown) {
            emscripten_cancel_main_loop();
            break;
        }

        if (!emu.isHalted) {
            while (ticks < emu.cpu.ticksPerFrame) {
                int cycles = emu.tick();
                if (cycles == 0) {
                    shutdown = true;
                    break;
                }

                if (emu.hitVBlank()) {
                    render(ctx->renderer, ctx->texture);
                    emu.cpu.apu.readSamples();
                    if (SDL_GetAudioDeviceStatus(ctx->audio) == SDL_AUDIO_PLAYING)
                        SDL_QueueAudio(ctx->audio, emu.cpu.apu.audioBuffer.data(), emu.cpu.apu.audioBuffer.size() * 2);
                }

                ticks += cycles;
            }
            ticks -= emu.cpu.ticksPerFrame;
        } else {
            render(ctx->renderer, ctx->texture);
        }
    }
}

int main(int argc, char* argv[]) {
    std::shared_ptr<StdSink> stdSink = std::make_shared<StdSink>(true);
    Logger::addSink(stdSink);
    Log(I, "Starting PhosGB\n");

    // init
    Log(I, "Initialising SDL\n");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        Log(F, "Could not init SDL. Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH * WEB_SCALE, HEIGHT * WEB_SCALE, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        Log(F, "Could not create SDL window. Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        Log(F, "Could not create SDL renderer. Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);
    if (texture == nullptr) {
        Log(F, "Could not create SDL texture. Error: %s\n", SDL_GetError());
        return 1;
    }

    Log(I, "Initialising core\n");
    // load file
    emu.isHalted = true;
    std::string ROM_Path = "";
    emu.load(ROM_Path);

    // init audio context
    SDL_AudioSpec spec;
    SDL_zero(spec);
    spec.freq = 44100;
    spec.format = AUDIO_S16;
    spec.channels = 2;
    spec.samples = 4096;
    SDL_AudioDeviceID audio = SDL_OpenAudioDevice(nullptr, 0, &spec, nullptr, 0);
    //SDL_PauseAudioDevice(audio, 0);

    //SDL_GL_SetSwapInterval(0);

    // main loop
    struct context ctx;
    ctx.window = window;
    ctx.renderer = renderer;
    ctx.texture = texture;
    ctx.audio = audio;
    emscripten_set_main_loop_arg(emulationLoop, &ctx, 60, 1);

    // shutdown
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    Log(I, "Shutting down emulator\n");
    return 0;
}