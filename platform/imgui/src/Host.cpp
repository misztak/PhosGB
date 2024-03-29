#include "Host.hpp"

Host::Host(SDL_Window* window, Emulator* emulator, SDL_AudioDeviceID deviceId)
    : mainTextureHandler(0), window(window), emulator(emulator), deviceId(deviceId),
      enableOverlay(true), requestOverlay(false), requestFileChooser(false) {}


bool Host::loadTexture(GLuint* textureHandler, u32 width, u32 height, u8* data) {
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

void Host::ImGuiInit(SDL_Window *window, void *glContext) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
#ifdef __EMSCRIPTEN__
    ImGui_ImplOpenGL3_Init("#version 100");
#else
    ImGui_ImplOpenGL3_Init(GLSL_VERSION);
#endif
    ImGui::GetStyle().WindowBorderSize = 0;
}

void Host::ImGuiDestroy() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void Host::showMainMenu() {
    ImGui::MenuItem("Menu", nullptr, false, false);
    if (ImGui::MenuItem("Open ROM")) {
        requestFileChooser = true;
    }
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
    if (ImGui::MenuItem("Pause", "H")) {
        emulator->pause();
    }
    #ifndef __EMSCRIPTEN__
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
    if (ImGui::MenuItem("Screenshot")) {
        const int size = WIDTH * HEIGHT * 4;
        const int scale = 3;
        std::vector<u8> pixelBuffer(size);
        glGetTextureImage(mainTextureHandler, 0, GL_RGBA, GL_UNSIGNED_BYTE, size, pixelBuffer.data());
        GLenum glError;
        if ((glError = glGetError()) != GL_NO_ERROR) {
            Log(W, "OpenGL error during screenshot creation (Code %u)\n", glError);
        } else {
            std::vector<u8> scaledBuffer(size * scale * scale, 0xFF);
            scaleFrame(pixelBuffer, scaledBuffer, scale);
            std::string fileName = emulator->currentFile + emulator->currentDateTime() + ".png";
            unsigned int error = lodepng::encode(fileName, scaledBuffer, WIDTH*scale, HEIGHT*scale);
            if (error)
                Log(W, "Lodepng encode error %u: %s\n", error, lodepng_error_text(error));
            else
                Log(I, "Created screenshot %s\n", fileName.c_str());
        }
    }
    #endif
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
        ImGui::Separator();
        // Misc ?
        ImGui::MenuItem("Use Boot ROM", "", &emulator->cpu.mmu.runBIOS);

        // End of Options Menu
        ImGui::EndMenu();
    }
}

void Host::showOverlay(bool *open, const char* extraMsg) {
    const float DISTANCE = 10.0f;
    static int corner = 0;
    ImGuiIO& io = ImGui::GetIO();
    if (corner != -1)
    {
        ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
        ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    }
    ImGui::SetNextWindowBgAlpha(0.5f); // Transparent background
    if (ImGui::Begin("Overlay", open,
            (corner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoNav))
    {
        if (emulator->cpu.ticksPerFrame == 280896 ||
            (!emulator->cpu.doubleSpeedMode && emulator->cpu.ticksPerFrame == 140448)) ImGui::Text("Fast-Forwarding x2");
        if (extraMsg) ImGui::Text("\n%s", extraMsg);
        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::MenuItem("Custom",       nullptr, corner == -1)) corner = -1;
            if (ImGui::MenuItem("Top-left",     nullptr, corner ==  0)) corner = 0;
            if (ImGui::MenuItem("Top-right",    nullptr, corner ==  1)) corner = 1;
            if (ImGui::MenuItem("Bottom-left",  nullptr, corner ==  2)) corner = 2;
            if (ImGui::MenuItem("Bottom-right", nullptr, corner ==  3)) corner = 3;
            if (open && ImGui::MenuItem("Close")) *open = false;
            ImGui::EndPopup();
        }
    }
    ImGui::End();
}

void Host::scaleFrame(std::vector<u8>& src, std::vector<u8>& dest, unsigned scale) {
    if (src.size() * scale * scale != dest.size()) {
        Log(W, "Vector size and scale value don't match\n");
        return;
    }
    unsigned counter = 0;
    switch (scale) {
        case 2: {
            for (unsigned y=0; y<HEIGHT; y++) {
                for (unsigned x=0; x<WIDTH; x++) {
                    for (int i=0; i<3; i++) {
                        dest[IX(x*2, y*2, WIDTH*scale) * 4 + i]     = src[counter + i];
                        dest[IX(x*2+1, y*2, WIDTH*scale) * 4 + i]   = src[counter + i];
                        dest[IX(x*2, y*2+1, WIDTH*scale) * 4 + i]   = src[counter + i];
                        dest[IX(x*2+1, y*2+1, WIDTH*scale) * 4 + i] = src[counter + i];
                    }
                    counter += 4;
                }
            }
            return; }
        case 3: {
            for (unsigned y=0; y<HEIGHT; y++) {
                for (unsigned x=0; x<WIDTH; x++) {
                    for (int i=0; i<3; i++) {
                        dest[IX(x*3, y*3, WIDTH*scale) * 4 + i]     = src[counter + i];
                        dest[IX(x*3+1, y*3, WIDTH*scale) * 4 + i]   = src[counter + i];
                        dest[IX(x*3, y*3+1, WIDTH*scale) * 4 + i]   = src[counter + i];
                        dest[IX(x*3+1, y*3+1, WIDTH*scale) * 4 + i] = src[counter + i];

                        dest[IX(x*3, y*3+2, WIDTH*scale) * 4 + i] = src[counter + i];
                        dest[IX(x*3+1, y*3+2, WIDTH*scale) * 4 + i] = src[counter + i];
                        dest[IX(x*3+2, y*3+2, WIDTH*scale) * 4 + i] = src[counter + i];
                        dest[IX(x*3+2, y*3+1, WIDTH*scale) * 4 + i] = src[counter + i];
                        dest[IX(x*3+2, y*3+0, WIDTH*scale) * 4 + i] = src[counter + i];
                    }
                    counter += 4;
                }
            }
            return; }
        default:
            Log(W, "Invalid scale factor %u\n", scale);
            return;
    }
}

void Host::loadFile(std::string& file) {
    emulator->load(file);
}
