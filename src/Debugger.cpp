#include <iostream>
#include <array>
#include "Debugger.h"

Debugger::Debugger() : enabled(false), show_demo_window(true), show_another_window(false) {}

void Debugger::initContext(SDL_Window* window, void* glContext) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init(GLSL_VERSION);
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

const int width = 160;
const int height = 144;
const size_t wow = width*height*4;

void memes(std::array<uint8_t, wow>& pixel) {
    uint8_t pixelBuffer[wow];
    memcpy(pixelBuffer, pixel.data(), wow);

    GLuint handler;
    glGenTextures(1, &handler);
    glBindTexture(GL_TEXTURE_2D, handler);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel.data());

    GLenum error;
    if ((error = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << error << std::endl;
        exit(1);
    }

    ImGui::Text("Serious shit inc");
    ImGui::Image((void*)(intptr_t)handler, ImVec2(160, 144));
    glBindTexture(GL_TEXTURE_2D, 0);
    ImGui::ShowMetricsWindow();
}

void Debugger::start(SDL_Window* window, std::array<uint8_t, 160*144*4>& pixel) {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &show_another_window);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        //ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        memes(pixel);
        ImGui::End();
    }

    // Rendering
    ImGui::Render();
}

void Debugger::draw() {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Debugger::stop() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}