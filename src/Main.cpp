#include <GL/glew.h>
#include <SDL.h>
#include <array>

#include "Common.h"
#include "Debugger.h"
#include "Display.h"
#include "Timer.h"

// TODO: Emulator class
#include "CPU.h"

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
// TODO: accuracy of this (maybe try 69905?)

bool initGL() {
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

int mockTick() {
    int a = std::rand();
    return a;
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

    std::array<u8, TEXTURE_SIZE> pixel = {};
    for (int i=1; i<=TEXTURE_SIZE; i++) {
        if (i % 4 == 0) pixel[i-1] = 255;
        else pixel[i-1] = 127;
    }
    CPU cpu;
    std::string filePath = "../../gb/Tetris.gb";
    cpu.init(filePath);

    if (!initGL()) {
        return 1;
    }

    IDisplay* host;
    Display display(pixel);
    Debugger debugger(window, glContext, &cpu, pixel);

    // Main loop
    host = &debugger;
    bool isDebugger = true;
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

            }
            host->processEvent(event);
        }

        // emulator tick
        while (ticks < ticksPerFrame) {
            mockTick();
            ticks++;
        }
        ticks -= ticksPerFrame;

        host->update(pixel);
        SDL_GL_MakeCurrent(window, glContext);

        glClear(GL_COLOR_BUFFER_BIT);
        host->render();

        SDL_GL_SwapWindow(window);

        frameTimer.syncFrame();
    }

    // Cleanup
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}