#include "IDisplay.h"

IDisplay::IDisplay(SDL_Window* window, Emulator* emulator)
    : mainTextureHandler(0), window(window), emulator(emulator) {}

bool IDisplay::initTexture(GLuint* textureHandler, u32 width, u32 height, u8* data) {
    glGenTextures(1, textureHandler);
    glBindTexture(GL_TEXTURE_2D, *textureHandler);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return !hasGLError();
}

bool IDisplay::loadTexture(GLuint textureHandler, u32 width, u32 height) {
    if (textureHandler != 0) {
        glLoadIdentity();
        glTranslatef(0.0f,0.0f,0.0f);
        glBindTexture(GL_TEXTURE_2D, textureHandler);

        glBegin(GL_QUADS);
        glTexCoord2f(0.f, 0.f); glVertex2f(0.f, 0.f);
        glTexCoord2f(1.f, 0.f); glVertex2f(width, 0.f);
        glTexCoord2f(1.f, 1.f); glVertex2f(width, height);
        glTexCoord2f(0.f, 1.f); glVertex2f(0.f, height);
        glEnd();
    }
    return !hasGLError();
}

bool IDisplay::hasGLError() {
    GLenum error;
    if ((error = glGetError()) != GL_NO_ERROR) {
        printf("OpenGL Error: %u\n", error);
        return true;
    }
    return false;
}

void IDisplay::ImGuiInit(SDL_Window *window, void *glContext) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
#if __APPLE__
    ImGui_ImplOpenGL2_Init();
#else
    ImGui_ImplOpenGL3_Init(GLSL_VERSION);
#endif
    ImGui::GetStyle().WindowBorderSize = 0;
}

void IDisplay::ImGuiDestroy() {
#if __APPLE__
    ImGui_ImplOpenGL2_Shutdown();
#else
    ImGui_ImplOpenGL3_Shutdown();
#endif
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}
