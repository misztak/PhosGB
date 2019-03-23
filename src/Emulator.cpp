#include "Emulator.h"

Emulator::Emulator(): isHalted(false), error(false) {}

bool Emulator::load(std::string& romPath) {
    return cpu.init(romPath);
}

u32 Emulator::tick() {
    return 1;
}

void Emulator::stop() {
    isHalted = !isHalted;
    if (isHalted) {
        printf("Stopped execution\n");
    } else {
        printf("Resumed execution\n");
    }
}

u8* Emulator::getDisplayState() {
    return nullptr;
}
