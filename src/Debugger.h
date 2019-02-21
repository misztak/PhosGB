#ifndef PHOS_DEBUGGER_H
#define PHOS_DEBUGGER_H

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include <GL/glew.h>
#include <SDL.h>
#include <array>

#if __APPLE__
#include "imgui_impl_opengl2.h"
#else
#define GLSL_VERSION "#version 130"
#include "imgui_impl_opengl3.h"
#endif

#define WIDTH 160
#define HEIGHT 144
#define SCALE 2
#define SCALED_WIDTH (WIDTH * SCALE)
#define SCALED_HEIGHT (HEIGHT * SCALE)
#define TEXTURE_SIZE (WIDTH * HEIGHT * 4)

class Debugger {
private:
    bool enabled;
public:
    GLuint textureHandler;
    bool show_demo_window;
    bool show_another_window;
public:
    Debugger();
    void initContext(SDL_Window* window, void* glContext, std::array<uint8_t, TEXTURE_SIZE>& pixel);
    void processEvent(SDL_Event& event);
    bool isEnabled();
    void toggle();
    void start(SDL_Window* window);
    void emulatorView();
    void draw();
    void stop();
};


#endif //PHOS_DEBUGGER_H
