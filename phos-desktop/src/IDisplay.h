#ifndef PHOS_IDISPLAY_H
#define PHOS_IDISPLAY_H

#include <GL/glew.h>
#include <SDL.h>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#if __APPLE__
#define GLSL_VERSION "#version 150"
#else
#define GLSL_VERSION "#version 130"
#endif

#include "Common.h"
#include "Emulator.h"

constexpr int SCALE = 3;
constexpr int SCALED_WIDTH = WIDTH * SCALE;
constexpr int SCALED_HEIGHT = HEIGHT * SCALE;

class IDisplay {
public:
    IDisplay(SDL_Window* window, Emulator* emulator);
    virtual void update(u8* data) = 0;
    virtual void render() = 0;
    virtual void processEvent(SDL_Event& event) = 0;

    bool loadTexture(GLuint* textureHandler, u32 width, u32 height, u8* data);
    void showMainMenu();

    static void ImGuiInit(SDL_Window* window, void* glContext);
    static void ImGuiDestroy();
public:
    GLuint mainTextureHandler;
    SDL_Window* window;
    Emulator* emulator;
};

#endif //PHOS_IDISPLAY_H
