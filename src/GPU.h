#ifndef PHOS_GPU_H
#define PHOS_GPU_H

#include "Common.h"
#include "MMU.h"

class CPU;

enum GPU_MODE { HBLANK, VBLANK, READ_OAM, READ_BOTH };

class GPU {
public:
    bool hitVBlank;
public:
    GPU(CPU* cpu, MMU* mmu);
    void tick(u32 ticks);
    u8* getDisplayState();
private:
    CPU* cpu;
    MMU* mmu;
    GPU_MODE mode;
    int modeclock;
    int line;

    std::vector<u8> displayState;
};

#endif //PHOS_GPU_H
