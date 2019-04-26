#ifndef PHOS_DEBUGGER_H
#define PHOS_DEBUGGER_H

#include <SDL.h>
#include <GL/glew.h>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_memory_editor.h"

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
    bool nextStep;
    bool singleStepMode;
public:
    Debugger(SDL_Window* window, void* glContext, Emulator* emu);
    ~Debugger();
    void render() override;
    void processEvent(SDL_Event& event) override;
    void update(u8* data) override;
private:
    GLuint bgTextureHandler;
    GLuint VRAMTextureHandler;
private:
    void emulatorView(u8* data);
    void memoryView();
    void backgroundView();
    void VRAMView();
};


#endif //PHOS_DEBUGGER_H
