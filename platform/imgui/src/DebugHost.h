#ifndef PHOS_DEBUGHOST_H
#define PHOS_DEBUGHOST_H

#include "Host.h"

#include "imgui_memory_editor.h"
#include "DebugSink.h"

class DebugHost : public Host {
public:
    bool nextStep, singleStepMode, showLogWindow, showDemoWindow, showMemWindow, showBGWindow, showVRAMWindow,
         showPaletteWindow;
    DebugSink* sink;
public:
    DebugHost(SDL_Window* window, Emulator* emu, SDL_AudioDeviceID deviceId, DebugSink* sink = nullptr);
    ~DebugHost();
    void render() override;
    void processEvent(SDL_Event& event) override;
    void update(u8* data) override;

    bool checkBreakpoints();
private:
    GLuint bgTextureHandler;
    GLuint VRAMTextureHandler;
    GLuint TileTextureHandler;

    std::unordered_map<u16, bool> breakpoints;
private:
    void emulatorView(u8* data);
    void memoryView();
    void backgroundView();
    void VRAMView();
    void paletteView();
    void logView();

    void renderVRAMView(u16 offset);
};


#endif //PHOS_DEBUGHOST_H
