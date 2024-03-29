#include <SDL.h>
#include <GL/gl3w.h>

#include "Common.hpp"
#include "DebugHost.hpp"
#include "NormalHost.hpp"
#include "Timer.hpp"
#include "Emulator.hpp"

#if __APPLE__
    // GL 3.2
    const int majorVersion = 3;
    const int minorVersion = 2;
    const int compatFlag = SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
#else
    // GL 3.0
    const int majorVersion = 3;
    const int minorVersion = 0;
    const int compatFlag = 0;
#endif

void render(SDL_Window* window, Host* host, Emulator* emulator) {
    host->update(emulator->getDisplayState());

    glClear(GL_COLOR_BUFFER_BIT);
    host->render();

    SDL_GL_SwapWindow(window);
}

void handleJoypadInput(SDL_Event& event, Emulator& emulator) {
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
        emulator.handleInputDown(key);
    } else if (event.type == SDL_KEYUP) {
        emulator.handleInputUp(key);
    }
}

void resize(SDL_Window* window, bool wasInNormalMode) {
    if (wasInNormalMode)
        SDL_SetWindowSize(window, 1200, 900);
    else
        SDL_SetWindowSize(window, SCALED_WIDTH, SCALED_HEIGHT);
}

int main(int argc, char** argv) {
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }
    SDL_SetHintWithPriority(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0", SDL_HINT_OVERRIDE);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, compatFlag);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, majorVersion);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minorVersion);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Create window and openGL context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_Window* window = SDL_CreateWindow(
            "PhosGB",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            1200, 900,
            SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI );
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, glContext);
    SDL_GL_SetSwapInterval(0);  // Vsync

    bool err = gl3wInit() != 0;
    if (err) {
        fprintf(stderr, "Failed to initialize GL3W!\n");
        return 1;
    }

    // Create audio context
    SDL_AudioSpec spec;
    SDL_zero(spec);
    spec.freq = 44100;
    spec.format = AUDIO_S16;
    spec.channels = 2;
    spec.samples = 4096;
    SDL_AudioDeviceID deviceId = SDL_OpenAudioDevice(nullptr, 0, &spec, nullptr, 0);

    std::shared_ptr<StdSink> stdSink = std::make_shared<StdSink>(false);
    std::shared_ptr<DebugSink> debugSink = std::make_shared<DebugSink>(true);
    Logger::addSink(stdSink);
    Logger::addSink(debugSink);

    Emulator emulator;
    std::string filePath = "../gb/";

    // GAMES
    //filePath.append("KirbyPinballLand.gb");
    //filePath.append("Tetris.gb");
    //filePath.append("F1-Race.gb");
    //filePath.append("Opus.gb");
    //filePath.append("TicTacToe.gb");
    //filePath.append("SuperMarioLand.gb");
    //filePath.append("PokemonRed.gb");
	//filePath.append("Zelda.gb");

	// CGB
	//filePath.append("PokemonYellow.gbc");
	//filePath.append("ZeldaDX.gbc");
	//filePath.append("ZeldaOracle.gbc");
	//filePath.append("WarioLand3.gbc");
	//filePath.append("DKC.gbc");
	//filePath.append("Aladdin.gbc");

    //filePath.append("blargg/instr_timing.gb");
    //filePath.append("mooneye/acceptance/halt_ime0_nointr_timing.gb");

    Host::ImGuiInit(window, glContext);
    Host* host;
    NormalHost display(window, &emulator, deviceId);
    DebugHost debugger(window, &emulator, deviceId, debugSink.get());

    if (!emulator.load(filePath)) {
        Log(W, "Failed to load hardcoded file\n");
        emulator.isHalted = true;
    }

    // Main loop
    host = &debugger;
    bool done = false;

    int ticks = 0;
    Timer frameTimer(PERFORMANCE);

    SDL_PauseAudioDevice(deviceId, 0);
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
            if (event.type == SDL_KEYUP) {
                if (event.key.keysym.scancode == SDL_SCANCODE_G) {
                    bool inDebugMode = host == &debugger;
                    inDebugMode ? host = &display : host = &debugger;
                    resize(window, !inDebugMode);
                }
                if (event.key.keysym.scancode == SDL_SCANCODE_F) {
                    emulator.cpu.ticksPerFrame = emulator.cpu.doubleSpeedMode ? 140448 : 70224;
                    host->requestOverlay = false;
                }
                if (event.key.keysym.scancode == SDL_SCANCODE_M) {
                    frameTimer.toggleMode();
                    SDL_GL_SetSwapInterval(frameTimer.getMode() == VSYNC ? 1 : 0);
                    Log(I, "Switched to mode %d\n", frameTimer.getMode());
                }
                if (event.key.keysym.scancode == SDL_SCANCODE_H) {
                    emulator.pause();
                }
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.scancode == SDL_SCANCODE_F) {
                    emulator.cpu.ticksPerFrame = emulator.cpu.doubleSpeedMode ? 280896 : 140448;
                    host->requestOverlay = true;
                }
            }
            handleJoypadInput(event, emulator);
            host->processEvent(event);
        }
        if (done) break;

        // emulator tick
        if (!emulator.isHalted) {
            while (ticks < emulator.cpu.ticksPerFrame) {
                int cycles = emulator.tick();
                if (cycles == 0) {
                    done = true;
                    break;
                }

                if (emulator.hitVBlank()) {
                    // Under normal circumstances the display should update at the start of every VBLANK period.
                    render(window, host, &emulator);
                    emulator.cpu.apu.readSamples();
                    if (SDL_GetAudioDeviceStatus(deviceId) == SDL_AUDIO_PLAYING && !host->requestOverlay)
                        SDL_QueueAudio(deviceId, emulator.cpu.apu.audioBuffer.data(),
                                       emulator.cpu.apu.audioBuffer.size() * 2);
                }

                ticks += cycles;

                // exit early if not in debugger
                if (host != &debugger) continue;

                if (debugger.checkBreakpoints()) {
                    ticks = emulator.cpu.ticksPerFrame;
                    Log(D, "Hit breakpoint at pc 0x%4X\n", emulator.cpu.r.pc);
                    break;
                }

                if (debugger.singleStepMode) {
                    debugger.nextStep = false;
                    emulator.isHalted = true;
                    render(window, host, &emulator);
                    ticks = emulator.cpu.ticksPerFrame;
                    Log(D, "Step\n");
                    break;
                }
            }
            ticks -= emulator.cpu.ticksPerFrame;
        } else {
            // Just keep rendering if the debugger has halted execution or if a fatal error has occurred.
            render(window, host, &emulator);
        }

        frameTimer.syncFrame();
    }

    // Save state if cartridge has persistent storage (battery buffered SRAM or flash ROM)
    emulator.shutdown();
    // Cleanup
    Host::ImGuiDestroy();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
