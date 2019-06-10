#ifndef PHOS_MMU_H
#define PHOS_MMU_H

#include <fstream>
#include <algorithm>
#include <map>
#include <memory>

#include "Common.h"
#include "MBC.h"

constexpr int BIOS_SIZE     = 256;
constexpr int ROM_BANK_SIZE = 16384;
constexpr int RAM_BANK_SIZE = 8192;
constexpr int WRAM_SIZE     = 8192;
constexpr int VRAM_SIZE     = 8192;
constexpr int OAM_SIZE      = 160;
constexpr int IO_SIZE       = 128;
constexpr int ZRAM_SIZE     = 128;

enum FileType { BIOS, ROM, SRAM };

class CPU;
class GPU;

class MMU {
public:
    MMU();
    bool init(std::string& romPath, std::string& biosPath);

    u8 readByte(u16 address);
    u16 readWord(u16 address);
    void writeByte(u16 address, u8 value);
    void writeWord(u16 address, u16 value);

    void printCartridgeInfo(std::vector<u8>& buffer);

    void saveState(std::ofstream& outfile);
    void loadState(std::vector<u8>& buffer);
public:
    CPU* cpu;
    GPU* gpu;

    bool inBIOS;
    bool runBIOS;

    std::vector<u8> BIOS;
    std::vector<u8> ROM_0;
    std::vector<u8> ROM;
    std::vector<u8> RAM;
    std::vector<u8> WRAM;
    std::vector<u8> IO;
    std::vector<u8> ZRAM;
    std::vector<u8> VRAM;
    std::vector<u8> OAM;

    std::string cartridgeTitle;
    std::map<u8, std::string> cartridgeTypes;
    std::map<u8, int> ROMSizeTypes;
    std::map<u8, int> RAMSizeTypes;

    std::unique_ptr<MBC> mbc;
private:
    bool loadFile(std::string& path, FileType fileType, std::vector<u8>& buffer);
    void initTables();
};

#endif //PHOS_MMU_H
