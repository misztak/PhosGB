#include "IDisplay.h"

IDisplay::IDisplay(SDL_Window* window, Emulator* emulator)
    : mainTextureHandler(0), window(window), emulator(emulator) {}


bool IDisplay::loadTexture(GLuint* textureHandler, u32 width, u32 height, u8* data) {
    glGenTextures(1, textureHandler);
    glBindTexture(GL_TEXTURE_2D, *textureHandler);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLenum error;
    if ((error = glGetError()) != GL_NO_ERROR) {
        printf("OpenGL Error: %u\n", error);
        return false;
    }
    return true;
}

void IDisplay::ImGuiInit(SDL_Window *window, void *glContext) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init(GLSL_VERSION);
    ImGui::GetStyle().WindowBorderSize = 0;
}

void IDisplay::ImGuiDestroy() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}
