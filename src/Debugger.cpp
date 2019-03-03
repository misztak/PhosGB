#include "Debugger.h"

Debugger::Debugger(SDL_Window* w, void* glContext, std::array<uint8_t, TEXTURE_SIZE>& pixel) {
    textureHandler = 0;
    show_demo_window = false;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    window = w;
    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
#if __APPLE__
    ImGui_ImplOpenGL2_Init();
#else
    ImGui_ImplOpenGL3_Init(GLSL_VERSION);
#endif
    initTexture(pixel);
}

Debugger::~Debugger() {
    if (textureHandler != 0) {
        glDeleteTextures(1, &textureHandler);
    }
#if __APPLE__
    ImGui_ImplOpenGL2_Shutdown();
#else
    ImGui_ImplOpenGL3_Shutdown();
#endif
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void Debugger::processEvent(SDL_Event& event) {
    ImGui_ImplSDL2_ProcessEvent(&event);
}

void Debugger::update(std::array<uint8_t, TEXTURE_SIZE>& pixel) {
#if __APPLE__
    ImGui_ImplOpenGL2_NewFrame();
#else
    ImGui_ImplOpenGL3_NewFrame();
#endif
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel.data());

    if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);
    emulatorView();
    static MemoryEditor editor;
    ImGui::Begin("Hex Editor");
    editor.DrawContents(pixel.data(), TEXTURE_SIZE);
    ImGui::End();

    // Rendering
    ImGui::Render();
}

void Debugger::emulatorView() {
    loadMainTexture();

    ImGui::Begin("Emulator");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Image((void*)(intptr_t)textureHandler, ImVec2(SCALED_WIDTH, SCALED_HEIGHT));
    ImGui::ShowMetricsWindow();
    ImGui::End();
}

void Debugger::render() {
#if __APPLE__
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
#else
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
}