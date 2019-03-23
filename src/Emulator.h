#ifndef PHOS_EMULATOR_H
#define PHOS_EMULATOR_H

#include "Common.h"
#include "CPU.h"

class Emulator {
public:
    Emulator();
    bool load(std::string& romPath);
    u32 tick();
    void stop();
    u8* getDisplayState();
public:
    bool isHalted;
    bool error;
    CPU cpu;

};

#endif //PHOS_EMULATOR_H
