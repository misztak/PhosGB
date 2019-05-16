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
    void shutdown();
    u8* getDisplayState();
    bool hitVBlank();
    void handleInputDown(u8 key);
    void handleInputUp(u8 key);
public:
    bool isHalted;
    bool isDead;
    CPU cpu;
    std::string currentFile;
    std::vector<std::string> recentFiles;
};

#endif //PHOS_EMULATOR_H
