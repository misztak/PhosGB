#ifndef PHOS_MMU_H
#define PHOS_MMU_H

#include <fstream>
#include <algorithm>
#include <map>

#include "Common.h"

constexpr int BIOS_SIZE     = 256;
constexpr int ROM_BANK_SIZE = 16384;
constexpr int RAM_BANK_SIZE = 8192;
constexpr int WRAM_SIZE     = 8192;
constexpr int VRAM_SIZE     = 8192;
constexpr int OAM_SIZE      = 160;
constexpr int IO_SIZE       = 128;
constexpr int ZRAM_SIZE     = 128;

class MMU {
public:
    MMU();
    bool init(std::string& romPath, std::string& biosPath);

    u8 readByte(u16 address);
    u16 readWord(u16 address);
    void writeByte(u16 address, u8 value);
    void writeWord(u16 address, u16 value);
public:
    bool inBIOS;

    std::vector<u8> BIOS;
    std::vector<u8> ROM_0;
    std::vector<u8> ROM;
    std::vector<u8> RAM;
    std::vector<u8> WRAM;
    std::vector<u8> IO;
    std::vector<u8> ZRAM;
    std::vector<u8> VRAM;
    std::vector<u8> OAM;

    std::map<u8, std::string> cartridgeTypes;
    std::map<u8, int> ROMSizeTypes;
    std::map<u8, int> RAMSizeTypes;
private:
    bool loadFile(std::string& path, bool isBIOS, std::vector<u8>& buffer);

    u8 readBankedROM(u16 address);
    u8 readBankedRAM(u16 address);
    void writeBankedRAM(u16 address, u8 value);
    void initTables();
};

#endif //PHOS_MMU_H
