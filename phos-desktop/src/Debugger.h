#ifndef PHOS_DEBUGGER_H
#define PHOS_DEBUGGER_H

#include "IDisplay.h"

#include "imgui_memory_editor.h"
#include "DebuggerLog.h"

class Debugger : public IDisplay {
public:
    bool show_demo_window;
    bool nextStep;
    bool singleStepMode;
    bool showLogWindow;
public:
    Debugger(SDL_Window* window, Emulator* emu, SDL_AudioDeviceID deviceId);
    ~Debugger();
    void render() override;
    void processEvent(SDL_Event& event) override;
    void update(u8* data) override;
private:
    GLuint bgTextureHandler;
    GLuint VRAMTextureHandler;
    GLuint TileTextureHandler;
private:
    void emulatorView(u8* data);
    void memoryView();
    void backgroundView();
    void VRAMView();
    void logView();
};


#endif //PHOS_DEBUGGER_H
