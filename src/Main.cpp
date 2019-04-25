#include <SDL.h>
#include <GL/glew.h>

#include "Common.h"
#include "Debugger.h"
#include "Display.h"
#include "Timer.h"
#include "Emulator.h"

#if __APPLE__
    // GL 2.2
    const int majorVersion = 2;
    const int minorVersion = 2;
    const int profileMask = 0;
#else
    // GL 3.0
    const int majorVersion = 3;
    const int minorVersion = 0;
    const int profileMask = SDL_GL_CONTEXT_PROFILE_CORE;
#endif

constexpr int ticksPerFrame = 70224;

bool initGL() {
    // TODO: stop using fixed function pipeline
    glViewport(0, 0, SCALED_WIDTH, SCALED_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, WIDTH, HEIGHT, 0.0, 1.0, -1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(0,0,0,255);
    glEnable(GL_TEXTURE_2D);

    GLenum error;
    if ((error = glGetError()) != GL_NO_ERROR) {
        printf("OpenGL Error: %u", error);
        return false;
    }
    return true;
}

void render(SDL_Window* window, SDL_GLContext* glContext, IDisplay* host, Emulator* emulator) {
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

int main(int argc, char** argv) {
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, profileMask);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, majorVersion);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minorVersion);

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
    SDL_Window* window = SDL_CreateWindow(
            "PhosGB",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            1200, 900,
            SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE );
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(0); // Vsync

    bool err = glewInit() != GLEW_OK;
    if (err) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    Emulator emulator;
    std::string filePath = "../../gb/Tetris.gb";
    //std::string filePath = "../../gb/blargg/cpu_instrs/02.gb";
    if (!emulator.load(filePath)) {
        fprintf(stderr, "Failed to load BootROM or Cartridge\n");
        return 2;
    }

    if (!initGL()) {
        return 1;
    }

    IDisplay* host;
    Display display(emulator.getDisplayState());
    Debugger debugger(window, glContext, &emulator);

    // Main loop
    host = &debugger;
    bool isDebugger = true;
    if (isDebugger && debugger.singleStepMode) {
        emulator.isHalted = true;
    }
    bool done = false;

    int ticks = 0;
    Timer frameTimer(ACCURACY);

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
                    render(window, &glContext, host, &emulator);
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
                    render(window, &glContext, host, &emulator);
                    ticks = ticksPerFrame;
                    printf("Step\n");
                    break;
                }
            }
            ticks -= ticksPerFrame;
        } else {
            // Just keep rendering if the debugger has halted execution or if a fatal error has occurred.
            render(window, &glContext, host, &emulator);
        }

        frameTimer.syncFrame();
    }

    // Cleanup
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
