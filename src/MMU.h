#ifndef PHOS_MMU_H
#define PHOS_MMU_H

#include <fstream>
#include <cstring>

#include "Common.h"

constexpr int INTERNAL_ROM_SIZE = 65536;
constexpr int BIOS_SIZE = 256;
constexpr int ROM0_SIZE = 16384;
constexpr int ROM1_SIZE = 16384;
constexpr int WRAM_SIZE = 8192;
constexpr int ERAM_SIZE = 8192;
constexpr int ZRAM_SIZE = 128;
constexpr int IO_SIZE = 128;

class MMU {
public:
    MMU();

    u8 readByte(u16 address);
    void writeByte(u16 address, u8 value);
    u16 readWord(u16 address);
    void writeWord(u16 address, u16 value);

    bool loadROM(std::string& filename, bool isBIOS = false);
public:
    bool inBIOS;
    bool fatalError;
    u8 bios[BIOS_SIZE];
    u8 rom0[ROM0_SIZE];
    u8 rom1[ROM1_SIZE];
    u8 workingRAM[WRAM_SIZE];
    u8 externalRAM[ERAM_SIZE];
    u8 mappedIO[IO_SIZE];
    u8 zeroPageRAM[ZRAM_SIZE];
private:
    typedef u8 (MMU::*Memory)(const u16 address);
    Memory memoryMap[16];

    u8 readROM0(u16 address);
    u8 readROM1(u16 address);
    u8 readWRAM(u16 address);
    u8 readWRAMshadow(u16 address);
    u8 readERAM(u16 address);
    u8 readZRAM(u16 address);
    u8 readVRAM(u16 address);
    // TODO: IO Control
    // TODO: OAM
};

#endif //PHOS_MMU_H
