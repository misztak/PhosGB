#include "Emulator.h"

Emulator::Emulator(): isHalted(false), isDead(false) {}

bool Emulator::load(std::string& romPath) {
    isHalted = false;
    isDead = false;
    return cpu.init(romPath);
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
