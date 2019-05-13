#include "Debugger.h"

Debugger::Debugger(SDL_Window* w, Emulator* emu) :
    IDisplay(w, emu), show_demo_window(true), nextStep(false), singleStepMode(false),
    bgTextureHandler(0), VRAMTextureHandler(0) {

    initTexture(&mainTextureHandler, WIDTH, HEIGHT, emulator->getDisplayState());
    initTexture(&bgTextureHandler, 256, 256, emulator->cpu.gpu.getBackgroundState());
    initTexture(&VRAMTextureHandler, 256, 192, emulator->cpu.gpu.getTileData());
}

Debugger::~Debugger() {
    if (mainTextureHandler != 0) {
        glDeleteTextures(1, &mainTextureHandler);
    }
    if (bgTextureHandler != 0) {
        glDeleteTextures(1, &bgTextureHandler);
    }
    if (VRAMTextureHandler != 0) {
        glDeleteTextures(1, &VRAMTextureHandler);
    }
}

void Debugger::processEvent(SDL_Event& event) {
    ImGui_ImplSDL2_ProcessEvent(&event);
}

void Debugger::update(u8* data) {
#if __APPLE__
    ImGui_ImplOpenGL2_NewFrame();
#else
    ImGui_ImplOpenGL3_NewFrame();
#endif
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();

    if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);
    emulatorView(data);
    memoryView();
    backgroundView();
    VRAMView();

    // Rendering
    ImGui::Render();
}

void Debugger::emulatorView(u8* data) {
    loadTexture(mainTextureHandler, WIDTH, HEIGHT);
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
    loadTexture(bgTextureHandler, 256, 256);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, emulator->cpu.gpu.getBackgroundState());

    ImGui::Begin("Background");
    ImGui::Image((void*)(intptr_t)bgTextureHandler, ImVec2(256, 256));
    ImGui::End();
}

void Debugger::VRAMView() {
    loadTexture(VRAMTextureHandler, 256, 192);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 192, 0, GL_RGBA, GL_UNSIGNED_BYTE, emulator->cpu.gpu.getTileData());

    ImGui::Begin("VRAM Viewer");
    ImGui::Image((void*)(intptr_t)VRAMTextureHandler, ImVec2(256, 192));
    ImGui::End();
}

void Debugger::render() {
#if __APPLE__
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
#else
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
}