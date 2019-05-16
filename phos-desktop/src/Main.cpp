#include <SDL.h>
#include <GL/glew.h>

#include "Common.h"
#include "Debugger.h"
#include "Display.h"
#include "Timer.h"
#include "Emulator.h"

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

constexpr int ticksPerFrame = 70224;

void render(SDL_Window* window, SDL_GLContext& glContext, IDisplay* host, Emulator* emulator) {
    host->update(emulator->getDisplayState());
    SDL_GL_MakeCurrent(window, glContext);

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

void resize(SDL_Window* window, bool isDebugger) {
    if (isDebugger) {
        SDL_SetWindowSize(window, 1200, 900);
        ImGui::GetStyle().WindowPadding = ImVec2(1, 1);
        ImGui::GetStyle().WindowRounding = 5;
    } else {
        SDL_SetWindowSize(window, SCALED_WIDTH, SCALED_HEIGHT);
        ImGui::GetStyle().WindowPadding = ImVec2(0, 0);
        ImGui::GetStyle().WindowRounding = 0;
    }
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

int main(int argc, char** argv) {
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
        printf("Error: %s\n", SDL_GetError());
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
    SDL_GL_SetSwapInterval(0);  // Vsync

    bool err = glewInit() != GLEW_OK;
    if (err) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    Emulator emulator;
    std::string filePath = "../../gb/";

    // GAMES
    //filePath.append("KirbyPinballLand.gb");
    //filePath.append("Tetris.gb");
    //filePath.append("F1-Race.gb");
    //filePath.append("Opus.gb");
    //filePath.append("TicTacToe.gb");
    //filePath.append("SuperMarioLand.gb");
    filePath.append("Zelda.gb");

    //filePath.append("blargg/instr_timing.gb");
    //filePath.append("mooneye/acceptance/halt_ime0_nointr_timing.gb");

    if (!emulator.load(filePath)) {
        fprintf(stderr, "Failed to load BootROM or Cartridge\n");
        return 2;
    }

    IDisplay::ImGuiInit(window, glContext);
    IDisplay* host;
    Display display(window, &emulator);
    Debugger debugger(window, &emulator);

    // Main loop
    host = &debugger;
    bool isDebugger = true;
    if (isDebugger && debugger.singleStepMode) {
        emulator.isHalted = true;
    }
    bool done = false;

    int ticks = 0;
    Timer frameTimer(PERFORMANCE);

    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
            if (event.type == SDL_KEYUP) {
                if (event.key.keysym.scancode == SDL_SCANCODE_G) {
                    if (isDebugger) {
                        isDebugger = false;
                        host = &display;
                    } else {
                        isDebugger = true;
                        host = &debugger;
                    }
                    resize(window, isDebugger);
                }
                if (event.key.keysym.scancode == SDL_SCANCODE_M) {
                    frameTimer.toggleMode();
                    SDL_GL_SetSwapInterval(frameTimer.getMode() == VSYNC ? 1 : 0);
                    printf("Switched to mode %d\n", frameTimer.getMode());
                }
                if (event.key.keysym.scancode == SDL_SCANCODE_H) {
                    emulator.toggle();
                }
            }
            handleJoypadInput(event, emulator);
            host->processEvent(event);
        }
        if (done) break;

        // emulator tick
        if (!emulator.isHalted) {
            while (ticks < ticksPerFrame) {
                int cycles = emulator.tick();
                if (cycles == 0) {
                    fprintf(stderr, "Encountered a fatal error during execution\n");
                    emulator.kill();
                    break;
                }

                if (emulator.hitVBlank()) {
                    // Under normal circumstances the display should update at the start of every VBLANK period.
                    render(window, glContext, host, &emulator);
                }

                ticks += cycles;

                // exit early if not in debugger
                if (!isDebugger) continue;

//                if (emulator.cpu.r.pc == 0x0100) {
//                    emulator.isHalted = true;
//                    debugger.singleStepMode = true;
//                    debugger.nextStep = false;
//                    ticks = ticksPerFrame;
//                    printf("Break\n");
//                    break;
//                }

                if (debugger.singleStepMode) {
                    debugger.nextStep = false;
                    emulator.isHalted = true;
                    render(window, glContext, host, &emulator);
                    ticks = ticksPerFrame;
                    printf("Step\n");
                    break;
                }
            }
            ticks -= ticksPerFrame;
        } else {
            // Just keep rendering if the debugger has halted execution or if a fatal error has occurred.
            render(window, glContext, host, &emulator);
        }

        frameTimer.syncFrame();
    }

    // Save state if cartridge has persistent storage
    emulator.shutdown();
    // Cleanup
    IDisplay::ImGuiDestroy();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
