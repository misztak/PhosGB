#include "Display.h"

Display::Display(SDL_Window* w, Emulator* emu, SDL_AudioDeviceID deviceId) : IDisplay(w, emu, deviceId) {
    loadTexture(&mainTextureHandler, WIDTH, HEIGHT, emulator->getDisplayState());
}

Display::~Display() {
    if (mainTextureHandler != 0) {
        glDeleteTextures(1, &mainTextureHandler);
    }
}

void Display::processEvent(SDL_Event &event) {
    if (event.type == SDL_KEYUP) {
        if (event.key.keysym.sym == SDLK_F5) {
            emulator->saveState();
        } else if (event.key.keysym.sym == SDLK_F6) {
            std::string quicksaveName = emulator->currentFile + "_Quicksave.state";
            emulator->loadState(quicksaveName);
        }
    }
    ImGui_ImplSDL2_ProcessEvent(&event);
}

void Display::update(u8* data) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();

    glBindTexture(GL_TEXTURE_2D, mainTextureHandler);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    bool open = true;
    ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoResize;

    ImGui::Begin("Emu", &open, ImVec2(SCALED_WIDTH, SCALED_HEIGHT), -1.0f, flags);
    ImGui::SetWindowSize(ImVec2(SCALED_WIDTH, SCALED_HEIGHT));
    ImGui::SetWindowPos(ImVec2(0, 0));
    ImGui::Image((void*)(intptr_t)mainTextureHandler, ImVec2(SCALED_WIDTH, SCALED_HEIGHT));
    ImGui::End();

    for (int i = 0; i < IM_ARRAYSIZE(ImGui::GetIO().MouseDown); i++) {
        if (ImGui::IsMouseClicked(i) && i == 1 && !requestOverlay) {
            ImGui::OpenPopup("menu_popup");
            break;
        }
    }
    if (ImGui::BeginPopup("menu_popup")) {
        showMainMenu();
        ImGui::EndPopup();
    }

    if (overlayEnable && requestOverlay) showOverlay(&overlayEnable);

    ImGui::Render();
}

void Display::render() {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

