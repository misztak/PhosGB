#ifndef PHOS_DEBUGGER_H
#define PHOS_DEBUGGER_H

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_memory_editor.h"
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
#include "Emulator.h"

class Debugger : public IDisplay {
public:
    SDL_Window* window;
    Emulator* emulator;
    bool show_demo_window;
public:
    Debugger(SDL_Window* window, void* glContext, Emulator* emu, std::array<uint8_t, TEXTURE_SIZE>& pixel);
    ~Debugger();
    void render() override;
    void processEvent(SDL_Event& event) override;
    void update(std::array<uint8_t, TEXTURE_SIZE>& pixels) override;
private:
    void emulatorView();
    void memoryView();
};


#endif //PHOS_DEBUGGER_H
