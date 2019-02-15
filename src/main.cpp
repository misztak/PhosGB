#include <GL/glew.h>
#include <SDL.h>
#include <array>

#include "Debugger.h"
#include "Display.h"

#if __APPLE__
    // GL 3.2 Core + GLSL 150
    const int compatFlags = SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
    const int minorVersion = 2;
#else
    // GL 3.0 + GLSL 130
    const int compatFlags = 0;
    const int minorVersion = 0;
#endif

int main(int argc, char** argv)
{
    // Debugger Test
    Debugger debugger;

    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, compatFlags);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    //SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    //SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
    SDL_Window* window = SDL_CreateWindow(
            "PhosGB",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            SCALED_WIDTH, SCALED_HEIGHT,
            SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE );
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
    bool err = glewInit() != GLEW_OK;
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    const int wow = 160*144*4;
    std::array<uint8_t, wow> pixel = {};
    int counter = 0;
    for (size_t i=0; i<pixel.size(); i+=4) {
        if (counter % 2 == 0) {
            pixel[i] = 255;
            pixel[i+1] = 255;
            pixel[i+2] = 255;
        } else {
            pixel[i] = 255;
            pixel[i+1] = 0;
            pixel[i+1] = 0;
        }
        pixel[i+3] = 255;
        counter++;
    }

    std::array<uint8_t, wow> pixel2 = {};
    for (uint8_t& p: pixel2) {
        p = 255;
    }

    Display display;
    if (!display.initGL()) {
        return 1;
    }
    if (!display.loadPixelArray(pixel)) {
        return 1;
    }
    debugger.initContext(window, gl_context, pixel2);

    // Main loop
    bool hostChanged = false;
    bool done = false;
    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (debugger.isEnabled()) debugger.processEvent(event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
            if (event.type == SDL_KEYDOWN) {
                debugger.toggle();
                hostChanged = true;
            }
        }

        if (hostChanged && debugger.isEnabled()) {
            SDL_SetWindowSize(window, 1200, 900);
            hostChanged = false;
        } else if (hostChanged && !debugger.isEnabled()) {
            SDL_SetWindowSize(window, SCALED_WIDTH, SCALED_HEIGHT);
            hostChanged = false;
        }

        if (debugger.isEnabled()) debugger.start(window);
        SDL_GL_MakeCurrent(window, gl_context);

        glClear(GL_COLOR_BUFFER_BIT);
        if (!debugger.isEnabled()) display.render();
        if (debugger.isEnabled()) debugger.draw();

        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    debugger.stop();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
