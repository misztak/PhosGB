#include <stdio.h>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <emscripten.h>
#include <SDL.h>
#include <SDL_opengles2.h>

#include "Common.hpp"
#include "DebugHost.hpp"
#include "NormalHost.hpp"
#include "Emulator.hpp"

SDL_Window* window = NULL;
SDL_GLContext glContext = NULL;
Host* host = NULL;
Emulator* emulator = NULL;
int ticks = 0;
SDL_AudioDeviceID deviceId;

void main_loop(void*);

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
        emulator->handleInputDown(key);
    } else if (event.type == SDL_KEYUP) {
        emulator->handleInputUp(key);
    }
}

void render() {
    host->update(emulator->getDisplayState());

    glClear(GL_COLOR_BUFFER_BIT);
    host->render();

    SDL_GL_SwapWindow(window);
}

int main(int, char**) {
    std::shared_ptr<StdSink> stdSink = std::make_shared<StdSink>(true);
    std::shared_ptr<DebugSink> debugSink = std::make_shared<DebugSink>(true);
    Logger::addSink(stdSink);
    Logger::addSink(debugSink);
    Log(I, "Initializing SDL\n");

    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER) != 0) {
        Log(F, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    const char* glsl_version = "#version 100";
    //const char* glsl_version = "#version 300 es";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window = SDL_CreateWindow("PhosGB Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        Log(F, "Failed to initialize WebGL context\n");
        return 1;
    }
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Create audio context
    SDL_AudioSpec spec;
    SDL_zero(spec);
    spec.freq = 44100;
    spec.format = AUDIO_S16;
    spec.channels = 2;
    spec.samples = 4096;
    deviceId = SDL_OpenAudioDevice(nullptr, 0, &spec, nullptr, 0);

    Log(I, "Initializing Imgui\n");

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = NULL;
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init(glsl_version);

    Log(I, "Initializing Host\n");
    host = new DebugHost(window, emulator, deviceId, debugSink.get());

    Log(I, "Initializing Emulator\n");
    emulator = new Emulator();
    std::string filePathDummy = "";
    if (!emulator->load(filePathDummy)) {
        Log(W, "Failed to load hardcoded file\n");
        emulator->isHalted = true;
    }

    SDL_PauseAudioDevice(deviceId, 1);
    // This function call won't return, and will engage in an infinite loop, processing events from the browser, and dispatching them.
    Log(I, "Starting main loop\n");
    emscripten_set_main_loop_arg(main_loop, NULL, 0, true);

    // This should never be reached
    return 0;
}

void main_loop(void* arg) {
    //ImGuiIO& io = ImGui::GetIO();
    IM_UNUSED(arg);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_KEYUP) {
            if (event.key.keysym.scancode == SDL_SCANCODE_F) {
                emulator->cpu.ticksPerFrame = emulator->cpu.doubleSpeedMode ? 140448 : 70224;
                host->requestOverlay = false;
            }
//            if (event.key.keysym.scancode == SDL_SCANCODE_M) {
//                frameTimer.toggleMode();
//                SDL_GL_SetSwapInterval(frameTimer.getMode() == VSYNC ? 1 : 0);
//                Log(I, "Switched to mode %d\n", frameTimer.getMode());
//            }
            if (event.key.keysym.scancode == SDL_SCANCODE_H) {
                emulator->pause();
            }
        }
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.scancode == SDL_SCANCODE_F) {
                emulator->cpu.ticksPerFrame = emulator->cpu.doubleSpeedMode ? 280896 : 140448;
                host->requestOverlay = true;
            }
        }
        handleJoypadInput(event);
        host->processEvent(event);
    }

    // emulator tick
    if (!emulator->isHalted) {
        while (ticks < emulator->cpu.ticksPerFrame) {
            int cycles = emulator->tick();

            if (emulator->hitVBlank()) {
                // Under normal circumstances the display should update at the start of every VBLANK period.
                render();
                emulator->cpu.apu.readSamples();
                if (SDL_GetAudioDeviceStatus(deviceId) == SDL_AUDIO_PLAYING && !host->requestOverlay)
                    SDL_QueueAudio(deviceId, emulator->cpu.apu.audioBuffer.data(),
                                   emulator->cpu.apu.audioBuffer.size() * 2);
            }

            ticks += cycles;
        }
        ticks -= emulator->cpu.ticksPerFrame;
    } else {
        // Just keep rendering if the debugger has halted execution or if a fatal error has occurred.
        render();
    }

    //frameTimer.syncFrame();
}