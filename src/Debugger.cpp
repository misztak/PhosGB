#include "Debugger.h"

Debugger::Debugger() :
    enabled(false),
    textureHandler(0),
    show_demo_window(true),
    show_another_window(false)
    {}

void Debugger::initContext(SDL_Window* window, void* glContext, std::array<uint8_t, TEXTURE_SIZE>& pixel) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init(GLSL_VERSION);

    glGenTextures(1, &textureHandler);
    glBindTexture(GL_TEXTURE_2D, textureHandler);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Debugger::processEvent(SDL_Event& event) {
    ImGui_ImplSDL2_ProcessEvent(&event);
}

bool Debugger::isEnabled() {
    return enabled;
}

void Debugger::toggle() {
    enabled = !enabled;
}

void Debugger::emulatorView() {
    if (textureHandler != 0) {
        glLoadIdentity();
        glTranslatef(0.0f,0.0f,0.0f);
        glBindTexture(GL_TEXTURE_2D, textureHandler);

        glBegin(GL_QUADS);
        glTexCoord2f(0.f, 0.f); glVertex2f(0.f, 0.f);
        glTexCoord2f(1.f, 0.f); glVertex2f(WIDTH, 0.f);
        glTexCoord2f(1.f, 1.f); glVertex2f(WIDTH, HEIGHT);
        glTexCoord2f(0.f, 1.f); glVertex2f(0.f, HEIGHT);
        glEnd();
    }

    GLenum error;
    if ((error = glGetError()) != GL_NO_ERROR) {
        printf("OpenGL Error: %u", error);
        exit(1);
    }
    ImGui::Begin("Emulator");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Image((void*)(intptr_t)textureHandler, ImVec2(SCALED_WIDTH, SCALED_HEIGHT));
    ImGui::ShowMetricsWindow();
    ImGui::End();
}

void Debugger::start(SDL_Window* window) {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();

    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
    }
    emulatorView();

    // Rendering
    ImGui::Render();
}

void Debugger::draw() {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Debugger::stop() {
    if (textureHandler != 0) {
        glDeleteTextures(1, &textureHandler);
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}