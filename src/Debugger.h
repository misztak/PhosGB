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

#include "IDisplay.h"

class Debugger : public IDisplay {
public:
    SDL_Window* window;
    bool show_demo_window;
public:
    Debugger(SDL_Window* window, void* glContext, std::array<uint8_t, TEXTURE_SIZE>& pixel);
    ~Debugger();
    void render() override;
    void processEvent(SDL_Event& event) override;
    void update(std::array<uint8_t, TEXTURE_SIZE>& pixels) override;
private:
    void emulatorView();
};


#endif //PHOS_DEBUGGER_H
