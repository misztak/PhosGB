#ifndef PHOS_EMULATOR_H
#define PHOS_EMULATOR_H

#include "Common.h"
#include "CPU.h"

class Emulator {
public:
    Emulator();
    bool load(std::string& romPath);
    u32 tick();
    void toggle();
    void kill();
    u8* getDisplayState();
    bool hitVBlank();
public:
    bool isHalted;
    bool isDead;
    CPU cpu;
};

#endif //PHOS_EMULATOR_H
