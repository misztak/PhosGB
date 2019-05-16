#include "Emulator.h"

Emulator::Emulator(): isHalted(false), isDead(false) {}

bool Emulator::load(std::string& romPath) {
    bool success = cpu.init(romPath);
    if (success) {
        isHalted = false;
        isDead = false;
        currentFile = romPath;
        if (std::find(recentFiles.begin(), recentFiles.end(), romPath) == recentFiles.end()) {
            recentFiles.push_back(romPath);
        }
    }
    return success;
}

u32 Emulator::tick() {
    return cpu.tick();
}

void Emulator::toggle() {
    if (isDead) {
        return;
    }
    isHalted = !isHalted;
    if (isHalted) {
        printf("Stopped execution\n");
    } else {
        printf("Resumed execution\n");
    }
}

void Emulator::kill() {
    isDead = true;
    isHalted = true;
}

void Emulator::shutdown() {
    u8 cartridgeType = cpu.mmu.ROM_0[0x147];
    if (cpu.mmu.cartridgeTypes[cartridgeType].find("RAM+BATTERY")) {
        std::string saveName = currentFile.substr(currentFile.find_last_of('/') + 1, currentFile.size());
        saveName.append(".sav");
        std::ofstream outfile(saveName, std::ios::out | std::ios::binary);
        outfile.write((char *) cpu.mmu.RAM.data(), cpu.mmu.RAM.size());
        printf("Saving RAM state to %s\n", saveName.c_str());
    }
}

u8* Emulator::getDisplayState() {
    return cpu.gpu.getDisplayState();
}

bool Emulator::hitVBlank() {
    if (cpu.gpu.hitVBlank) {
        cpu.gpu.hitVBlank = false;
        return true;
    } else {
        return false;
    }
}

void Emulator::handleInputDown(u8 key) {
    cpu.handleInputDown(key);
}

void Emulator::handleInputUp(u8 key) {
    cpu.handleInputUp(key);
}
