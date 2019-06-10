#include "IDisplay.h"

IDisplay::IDisplay(SDL_Window* window, Emulator* emulator, SDL_AudioDeviceID deviceId)
    : mainTextureHandler(0), window(window), emulator(emulator), deviceId(deviceId) {}


bool IDisplay::loadTexture(GLuint* textureHandler, u32 width, u32 height, u8* data) {
    glGenTextures(1, textureHandler);
    glBindTexture(GL_TEXTURE_2D, *textureHandler);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLenum error;
    if ((error = glGetError()) != GL_NO_ERROR) {
        Log(F, "OpenGL Error: %u\n", error);
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

void IDisplay::showMainMenu() {
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
            Log(I, "Successfully loaded save state %s\n", quicksaveName.c_str());
        else
            Log(W, "Failed to load save state %s\n", quicksaveName.c_str());
    }
    ImGui::Separator();
    if (ImGui::BeginMenu("Options")) {
        // Sound Control
        static bool soundDisabled = false;
        ImGui::MenuItem("Disable Sound", "", &soundDisabled);
        SDL_PauseAudioDevice(deviceId, soundDisabled);
        if (ImGui::BeginMenu("Disable Channels")) {
            ImGui::MenuItem("CH1", nullptr, &emulator->cpu.apu.masterEnable[0]);
            ImGui::MenuItem("CH2", nullptr, &emulator->cpu.apu.masterEnable[1]);
            ImGui::MenuItem("CH3", nullptr, &emulator->cpu.apu.masterEnable[2]);
            ImGui::MenuItem("CH4", nullptr, &emulator->cpu.apu.masterEnable[3]);
            ImGui::EndMenu();
        }
        ImGui::Separator();
        // Custom Palette
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
        ImGui::Separator();
        // Windows Size
        if (ImGui::BeginMenu("Window Size [TODO]")) {
            static bool selection[4] = {false, false, true, false};
            ImGui::MenuItem("1x1", nullptr, selection[0]);
            ImGui::MenuItem("2x2", nullptr, selection[1]);
            ImGui::MenuItem("3x3", nullptr, selection[2]);
            ImGui::MenuItem("4x4", nullptr, selection[3]);
            ImGui::EndMenu();
        }

        // End of Options Menu
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
