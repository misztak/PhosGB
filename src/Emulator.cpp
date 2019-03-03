#include <GL/glew.h>
#include <SDL.h>
#include <array>

#include "Debugger.h"
#include "Display.h"

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

int main(int argc, char** argv)
{
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
    {
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
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    bool err = glewInit() != GLEW_OK;
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    std::array<uint8_t, TEXTURE_SIZE> pixel = {};
    for (uint8_t& p: pixel) {
        p = 127;
    }

    if (!initGL()) {
        return 1;
    }

    IDisplay* host;
    Display display(pixel);
    Debugger debugger(window, gl_context, pixel);

    // Main loop
    host = &debugger;
    bool isDebugger = true;
    bool done = false;
    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
            if (event.type == SDL_KEYUP && event.key.keysym.scancode == SDL_SCANCODE_G) {
                if (isDebugger) {
                    isDebugger = false;
                    host = &display;
                } else {
                    isDebugger = true;
                    host = &debugger;
                }
            }
            host->processEvent(event);
        }

        host->update(pixel);
        SDL_GL_MakeCurrent(window, gl_context);

        glClear(GL_COLOR_BUFFER_BIT);
        host->render();

        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
