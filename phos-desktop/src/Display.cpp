#include "Display.h"

Display::Display(SDL_Window* w, Emulator* emu) : IDisplay(w, emu) {
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
        if (ImGui::IsMouseClicked(i) && i == 1) {
            ImGui::OpenPopup("menu_popup");
            break;
        }
    }
    if (ImGui::BeginPopup("menu_popup")) {
        showMenuPopup();
        ImGui::EndPopup();
    }

    ImGui::Render();
}

void Display::render() {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Display::showMenuPopup() {
    ImGui::MenuItem("Menu", nullptr, false, false);
    if (ImGui::MenuItem("Open ROM..")) {}
    if (ImGui::MenuItem("Reset ROM")) {
        emulator->load(emulator->currentFilePath);
    }
    if (ImGui::BeginMenu("Open Recent")) {
        for (std::string& recentFile : emulator->recentFiles) {
            if (ImGui::MenuItem(recentFile.c_str())) {
                emulator->load(recentFile);
                break;
            }
        }
        ImGui::EndMenu();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Save State", "F5")) {
        emulator->saveState();
    }
    if (ImGui::MenuItem("Load State", "F6")) {
        std::string quicksaveName = emulator->currentFile + "_Quicksave.state";
        if (emulator->loadState(quicksaveName))
            printf("Successfully loaded save state %s\n", quicksaveName.c_str());
        else
            printf("Failed to load save state %s\n", quicksaveName.c_str());
    }
    if (ImGui::BeginMenu("Options")) {
        if (ImGui::BeginMenu("Window Size")) {
            static bool selection[4] = {false, false, true, false};
            ImGui::MenuItem("1x1", nullptr, selection[0]);
            ImGui::MenuItem("2x2", nullptr, selection[1]);
            ImGui::MenuItem("3x3", nullptr, selection[2]);
            ImGui::MenuItem("4x4", nullptr, selection[3]);
            ImGui::EndMenu();
        }
        static bool paletteEnable = emulator->cpu.gpu.useCustomPalette;
        ImGui::MenuItem("Use Custom Palette", "", &paletteEnable);
        emulator->cpu.gpu.useCustomPalette = paletteEnable;
        if (ImGui::BeginMenu("Custom Palette")) {
            u8* p0 = emulator->cpu.gpu.customPalette[colors[0]].data();
            u8* p1 = emulator->cpu.gpu.customPalette[colors[1]].data();
            u8* p2 = emulator->cpu.gpu.customPalette[colors[2]].data();
            u8* p3 = emulator->cpu.gpu.customPalette[colors[3]].data();
            ImVec4 clrs[4] = {
                ImVec4(p0[0]/255.0f, p0[1]/255.0f, p0[2]/255.0f, 1.f),
                ImVec4(p1[0]/255.0f, p1[1]/255.0f, p1[2]/255.0f, 1.f),
                ImVec4(p2[0]/255.0f, p2[1]/255.0f, p2[2]/255.0f, 1.f),
                ImVec4(p3[0]/255.0f, p3[1]/255.0f, p3[2]/255.0f, 1.f),
            };
            ImGui::ColorEdit3("P1", (float*)&clrs[0], 0);
            ImGui::ColorEdit3("P2", (float*)&clrs[1], 0);
            ImGui::ColorEdit3("P3", (float*)&clrs[2], 0);
            ImGui::ColorEdit3("P4", (float*)&clrs[3], 0);
            for (int i=0; i<4; i++) {
                emulator->cpu.gpu.customPalette[colors[i]][0] = clrs[i].x * 255;
                emulator->cpu.gpu.customPalette[colors[i]][1] = clrs[i].y * 255;
                emulator->cpu.gpu.customPalette[colors[i]][2] = clrs[i].z * 255;
            }
            if (ImGui::Button("Reset")) {
                emulator->cpu.gpu.customPalette.clear();
                emulator->cpu.gpu.customPalette[colors[0]] = {224, 248, 208};
                emulator->cpu.gpu.customPalette[colors[1]] = {136, 192, 112};
                emulator->cpu.gpu.customPalette[colors[2]] = { 52, 104,  86};
                emulator->cpu.gpu.customPalette[colors[3]] = {  8,  24,  32};
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Colors"))
    {
        float sz = ImGui::GetTextLineHeight();
        for (int i = 0; i < ImGuiCol_COUNT; i++)
        {
            const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x+sz, p.y+sz), ImGui::GetColorU32((ImGuiCol)i));
            ImGui::Dummy(ImVec2(sz, sz));
            ImGui::SameLine();
            ImGui::MenuItem(name);
        }
        ImGui::EndMenu();
    }
}
