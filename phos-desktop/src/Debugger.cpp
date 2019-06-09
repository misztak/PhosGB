#include "Debugger.h"

Debugger::Debugger(SDL_Window* w, Emulator* emu) :
    IDisplay(w, emu), show_demo_window(false), nextStep(false), singleStepMode(false),
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
        if (ImGui::BeginMenu("Menu")) {
            showMainMenu();
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);
    emulatorView(data);
    memoryView();
    backgroundView();
    VRAMView();

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
    if (singleStepMode && !emulator->isDead) {
        emulator->isHalted = !nextStep;
    }
    if (ImGui::Button("Continue")) {
        singleStepMode = false;
        emulator->isHalted = false;
    }

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
    ImGui::Begin("Memory Editor");

    const char* items[] = {"ROM0", "ROM1", "WRAM", "ERAM", "ZRAM", "IO", "BIOS", "VRAM", "OAM"};
    static int currentItem = 0;
    ImGui::Combo("Location", &currentItem, items, IM_ARRAYSIZE(items));

    switch (currentItem) {
        case 0:
            editor.DrawContents(emulator->cpu.mmu.ROM_0.data(), ROM_BANK_SIZE);
            break;
        case 1:
            // TODO: show all banks
            editor.DrawContents(emulator->cpu.mmu.ROM.data(), ROM_BANK_SIZE);
            break;
        case 2:
            editor.DrawContents(emulator->cpu.mmu.WRAM.data(), WRAM_SIZE);
            break;
        case 3:
            // TODO: show all banks
            editor.DrawContents(emulator->cpu.mmu.RAM.data(), RAM_BANK_SIZE);
            break;
        case 4:
            editor.DrawContents(emulator->cpu.mmu.ZRAM.data(), ZRAM_SIZE);
            break;
        case 5:
            editor.DrawContents(emulator->cpu.mmu.IO.data(), IO_SIZE);
            break;
        case 6:
            editor.DrawContents(emulator->cpu.mmu.BIOS.data(), BIOS_SIZE);
            break;
        case 7:
            editor.DrawContents(emulator->cpu.mmu.VRAM.data(), VRAM_SIZE);
            break;
        case 8:
            editor.DrawContents(emulator->cpu.mmu.OAM.data(), OAM_SIZE);
            break;
        default:
            break;
    }

    ImGui::End();
}

void Debugger::backgroundView() {
    glBindTexture(GL_TEXTURE_2D, bgTextureHandler);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, emulator->cpu.gpu.getBackgroundState());

    ImGui::Begin("Background");
    ImGui::Image((void*)(intptr_t)bgTextureHandler, ImVec2(256, 256));
    ImGui::End();
}

void Debugger::VRAMView() {
    glBindTexture(GL_TEXTURE_2D, VRAMTextureHandler);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8*16, 8*24, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    const int tileSize = 16;
    int tileCounter = 0;
    for (int y=0; y<24; y++) {
        for (int x=0; x<16; x++) {
            glTexSubImage2D(GL_TEXTURE_2D, 0, x*8, y*8, 8, 8, GL_RGBA, GL_UNSIGNED_BYTE, emulator->cpu.gpu.getTileData(tileCounter++ * tileSize));
        }
    }

    ImGui::Begin("VRAM Viewer");
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
        ImGui::Text("Tile Address: 0x%4X", tileID * tileSize + 0x8000);

        glBindTexture(GL_TEXTURE_2D, TileTextureHandler);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, emulator->cpu.gpu.getTileData(tileID * tileSize));
        ImGui::Image((void*)(intptr_t)TileTextureHandler, ImVec2(8 * zoom, 8 * zoom));
        ImGui::EndTooltip();
    }

    ImGui::End();
}

void Debugger::render() {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}