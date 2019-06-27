#include "Debugger.h"

Debugger::Debugger(SDL_Window* w, Emulator* emu, SDL_AudioDeviceID deviceId, DebugSink* sink) :
    IDisplay(w, emu, deviceId),
    nextStep(false),
    singleStepMode(false),
    showLogWindow(true), showDemoWindow(false), showMemWindow(false), showBGWindow(true), showVRAMWindow(true),
    showPaletteWindow(false),
    sink(sink),
    bgTextureHandler(0), VRAMTextureHandler(0), TileTextureHandler(0) {

    loadTexture(&mainTextureHandler, WIDTH, HEIGHT, emulator->getDisplayState());
    loadTexture(&bgTextureHandler, 256, 256, emulator->cpu.gpu.getBackgroundState());
    loadTexture(&VRAMTextureHandler, 8*16, 8*24, nullptr);
    loadTexture(&TileTextureHandler, 64, 64, nullptr);
}

Debugger::~Debugger() {
    if (mainTextureHandler != 0)    glDeleteTextures(1, &mainTextureHandler);
    if (bgTextureHandler != 0)      glDeleteTextures(1, &bgTextureHandler);
    if (VRAMTextureHandler != 0)    glDeleteTextures(1, &VRAMTextureHandler);
    if (TileTextureHandler != 0)    glDeleteTextures(1, &TileTextureHandler);
}

void Debugger::processEvent(SDL_Event& event) {
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

void Debugger::update(u8* data) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();

    bool open = true;
    ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_MenuBar;
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    ImGui::Begin("DebuggerWindow", &open, ImVec2(w, h), -1.0f, flags);
    ImGui::SetWindowPos(ImVec2(0, 0));
    // Menu
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            showMainMenu();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Window")) {
            ImGui::MenuItem("Background Viewer", nullptr, &showBGWindow);
            ImGui::MenuItem("VRAM Viewer", nullptr, &showVRAMWindow);
            ImGui::MenuItem("Palette Viewer", nullptr, &showPaletteWindow);
            ImGui::MenuItem("Memory Editor", nullptr, &showMemWindow);
            ImGui::MenuItem("Debug Log", nullptr, &showLogWindow);
            ImGui::MenuItem("Overlay", nullptr, &enableOverlay);
            ImGui::MenuItem("Imgui Demo", nullptr, &showDemoWindow);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (requestFileChooser) ImGui::OpenPopup("FileChooser");
    ImGui::SetNextWindowSize(ImVec2(600, 400));
    if (ImGui::BeginPopupModal("FileChooser", nullptr, ImGuiWindowFlags_NoScrollbar)) {
        static FileChooser fileChooser;
        fileChooser.DrawWindow();


        ImGui::Separator();
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
            requestFileChooser = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Select")) {
            std::string file = fileChooser.GetSelected();
            if (file.empty()) {
                fileChooser.badFile = true;
            } else {
                ImGui::CloseCurrentPopup();
                requestFileChooser = false;
                emulator->load(file);
            }
        }
        if (fileChooser.badFile) {
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.f, 0.f, 1.f));
            ImGui::Text("Invalid File");
            ImGui::PopStyleColor();
        }
        ImGui::EndPopup();
    }

    emulatorView(data);
    if (showMemWindow) memoryView();
    if (showBGWindow) backgroundView();
    if (showVRAMWindow) VRAMView();
    if (showPaletteWindow) paletteView();
    if (showLogWindow) logView();
    if (showDemoWindow) ImGui::ShowDemoWindow(&showDemoWindow);
    if (enableOverlay && requestOverlay) showOverlay(&enableOverlay);

    ImGui::End();
    // Rendering
    ImGui::Render();
}

void Debugger::emulatorView(u8* data) {
    glBindTexture(GL_TEXTURE_2D, mainTextureHandler);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    ImGui::Begin("Emulator");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Image((void*)(intptr_t)mainTextureHandler, ImVec2(SCALED_WIDTH, SCALED_HEIGHT));
    //ImGui::ShowMetricsWindow();
    nextStep = ImGui::Button("Step");
    if (singleStepMode) {
        emulator->isHalted = !nextStep;
    }
    if (ImGui::Button("Continue")) {
        singleStepMode = false;
        emulator->isHalted = false;
    }

    ImGui::Text("GB Mode: %s", (emulator->cpu.gbMode == DMG) ? "DMG" : "CGB");
    ImGui::Text("Registers:");
    ImGui::Text("PC: 0x%04X", emulator->cpu.r.pc);
    ImGui::Text("SP: 0x%04X", emulator->cpu.r.sp);
    ImGui::Text("IME: %d", emulator->cpu.r.ime);
    ImGui::Text("AF: 0x%04X", emulator->cpu.r.af);
    ImGui::Text("BC: 0x%04X", emulator->cpu.r.bc);
    ImGui::Text("DE: 0x%04X", emulator->cpu.r.de);
    ImGui::Text("HL: 0x%04X", emulator->cpu.r.hl);
    ImGui::Text("Flags:");
    ImGui::Text("ZERO %d", (emulator->cpu.r.f & ZERO) != 0);
    ImGui::Text("ADD_SUB %d", (emulator->cpu.r.f & ADD_SUB) != 0);
    ImGui::Text("HALF_CARRY %d", (emulator->cpu.r.f & HALF_CARRY) != 0);
    ImGui::Text("CARRY %d", (emulator->cpu.r.f & CARRY) != 0);
    ImGui::End();
}

void Debugger::memoryView() {
    static MemoryEditor editor;
    ImGui::Begin("Memory Editor", &showMemWindow);

    const char* items[] = {"ROM0", "ROM", "WRAM", "SRAM", "ZRAM", "IO", "BIOS", "VRAM", "OAM", "CGB Palette"};
    static int currentItem = 0;
    ImGui::Combo("Location", &currentItem, items, IM_ARRAYSIZE(items));

    switch (currentItem) {
        case 0:
            editor.DrawContents(emulator->cpu.mmu.ROM_0.data(), emulator->cpu.mmu.ROM_0.size());
            break;
        case 1:
            editor.DrawContents(emulator->cpu.mmu.ROM.data(), emulator->cpu.mmu.ROM.size());
            break;
        case 2:
            editor.DrawContents(emulator->cpu.mmu.WRAM.data(), emulator->cpu.mmu.WRAM.size());
            break;
        case 3:
            editor.DrawContents(emulator->cpu.mmu.RAM.data(), emulator->cpu.mmu.RAM.size());
            break;
        case 4:
            editor.DrawContents(emulator->cpu.mmu.ZRAM.data(), emulator->cpu.mmu.ZRAM.size());
            break;
        case 5:
            editor.DrawContents(emulator->cpu.mmu.IO.data(), emulator->cpu.mmu.IO.size());
            break;
        case 6:
            editor.DrawContents(emulator->cpu.mmu.BIOS.data(), emulator->cpu.mmu.BIOS.size());
            break;
        case 7:
            editor.DrawContents(emulator->cpu.mmu.VRAM.data(), emulator->cpu.mmu.VRAM.size());
            break;
        case 8:
            editor.DrawContents(emulator->cpu.mmu.OAM.data(), emulator->cpu.mmu.OAM.size());
            break;
        case 9:
            editor.DrawContents(emulator->cpu.mmu.PaletteMemory.data(), emulator->cpu.mmu.PaletteMemory.size());
            break;
        default:
            break;
    }

    ImGui::End();
}

void Debugger::backgroundView() {
    glBindTexture(GL_TEXTURE_2D, bgTextureHandler);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, emulator->cpu.gpu.getBackgroundState());

    ImGui::Begin("Background", &showBGWindow);
    ImGui::Image((void*)(intptr_t)bgTextureHandler, ImVec2(256, 256));
    ImGui::End();
}

void Debugger::VRAMView() {


    ImGui::Begin("VRAM Viewer", &showVRAMWindow);
    if (ImGui::BeginTabBar("VRAMTabs")) {
        if (ImGui::BeginTabItem("Bank 1")) {
            renderVRAMView(0x0000);
            ImGui::EndTabItem();
        }
        if (emulator->cpu.gbMode == CGB) {
            if (ImGui::BeginTabItem("Bank 2")) {
                renderVRAMView(VRAM_BANK_SIZE);
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void Debugger::renderVRAMView(u16 offset) {
    glBindTexture(GL_TEXTURE_2D, VRAMTextureHandler);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8*16, 8*24, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    const int tileSize = 16;
    int tileCounter = 0;
    for (int y=0; y<24; y++) {
        for (int x=0; x<16; x++) {
            glTexSubImage2D(GL_TEXTURE_2D, 0, x*8, y*8, 8, 8, GL_RGBA, GL_UNSIGNED_BYTE,
                    emulator->cpu.gpu.getTileData(tileCounter++ * tileSize + offset));
        }
    }

    ImGui::Image((void*)(intptr_t)VRAMTextureHandler, ImVec2(8*16*2, 8*24*2));
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        float relX = io.MousePos.x - pos.x;
        float relY = io.MousePos.y - pos.y + 388;
        int tileColumn = (int) (relX / 16);
        int tileRow = (int) (relY / 16);
        int tileID = tileRow * 16 + tileColumn;
        float zoom = 8.f;
        if (tileID < 128) ImGui::Text("Tile ID: (Set #1: %d)", tileID);
        else if (tileID < 256) ImGui::Text("Tile ID: (Set #1: %d)\nTile ID: (Set #2: %d)", tileID, static_cast<char>(tileID));
        else ImGui::Text("Tile ID: (Set #2: %d)", tileID - 256);
        ImGui::Text("Tile Pos: (%d, %d)", tileColumn, tileRow);
        ImGui::Text("Tile Address: 0x%4X", tileID * tileSize + 0x8000 + offset);

        glBindTexture(GL_TEXTURE_2D, TileTextureHandler);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                emulator->cpu.gpu.getTileData(tileID * tileSize + offset));
        ImGui::Image((void*)(intptr_t)TileTextureHandler, ImVec2(8 * zoom, 8 * zoom));
        ImGui::EndTooltip();
    }
}

void Debugger::paletteView() {
    ImGui::Begin("Palette Viewer", &showPaletteWindow);

//    GLuint pHandlers[32] = {};
//    for (GLuint pHandler : pHandlers) {
//        glBindTexture(GL_TEXTURE_2D, pHandler);
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
//    }
    u32 palettePtr = 0;
    for (u32 t=0; t<2; t++) {
        for (u32 i=0; i<8; i++) {
            u16 color1 = emulator->cpu.mmu.PaletteMemory[palettePtr] | (emulator->cpu.mmu.PaletteMemory[palettePtr+1] << 8);
            u16 color2 = emulator->cpu.mmu.PaletteMemory[palettePtr+2] | (emulator->cpu.mmu.PaletteMemory[palettePtr+3] << 8);
            u16 color3 = emulator->cpu.mmu.PaletteMemory[palettePtr+4] | (emulator->cpu.mmu.PaletteMemory[palettePtr+5] << 8);
            u16 color4 = emulator->cpu.mmu.PaletteMemory[palettePtr+6] | (emulator->cpu.mmu.PaletteMemory[palettePtr+7] << 8);
            palettePtr += 8;
            ImGui::Text("0x%04X", color1); ImGui::SameLine();
            ImGui::Text("0x%04X", color2); ImGui::SameLine();
            ImGui::Text("0x%04X", color3); ImGui::SameLine();
            ImGui::Text("0x%04X", color4);
        }
        ImGui::NewLine();
    }

    ImGui::End();
}

void Debugger::logView() {
    if (sink) sink->Draw("Log", &showLogWindow);
}

void Debugger::render() {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}