#ifndef PHOS_MMU_H
#define PHOS_MMU_H

#include <fstream>

#include "Common.h"

constexpr int MEMORY_SIZE = 65536;

class MMU {
public:
    u8 readByte(u16 address);
    void writeByte(u16 address, u8 value);
    u16 readWord(u16 address);
    void writeWord(u16 address, u16 value);

    bool loadROM(std::string& filename);
    u8* getMemory();
private:
    u8 memory[MEMORY_SIZE];

};

#endif //PHOS_MMU_H
