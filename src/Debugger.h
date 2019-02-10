#ifndef PHOS_DEBUGGER_H
#define PHOS_DEBUGGER_H

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <GL/gl3w.h>
#include <SDL.h>

#if __APPLE__
#define GLSL_VERSION "#version 150"
#else
#define GLSL_VERSION "#version 130"
#endif

class Debugger {
private:
    bool enabled;
    bool show_demo_window;
    bool show_another_window;
public:
    ImGuiIO io;
    Debugger();
    void initContext(SDL_Window* window, void* glContext);
    void processEvent(SDL_Event& event);
    bool isEnabled();
    void toggle();
    void start(SDL_Window* window, ImVec4& clear_color);
    void draw();
    void stop();
};


#endif //PHOS_DEBUGGER_H
