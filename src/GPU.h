#ifndef PHOS_GPU_H
#define PHOS_GPU_H

#include "Common.h"
#include "MMU.h"

class CPU;

enum GPU_MODE { HBLANK, VBLANK, READ_OAM, READ_BOTH };

const u8 colors[] { 255, 192, 96, 0 };

constexpr int VRAM_SIZE = 8192;
constexpr int OAM_SIZE = 160;

// GPU Registers
constexpr u16 LCD_CONTROL = 0xFF40;
constexpr u16 LCDC_STATUS = 0xFF41;
constexpr u16 SCROLL_Y = 0xFF42;
constexpr u16 SCROLL_X = 0xFF43;
constexpr u16 LCDC_Y_COORDINATE = 0xFF44;
constexpr u16 LY_COMPARE = 0xFF45;
constexpr u16 WINDOW_Y = 0xFF4A;
constexpr u16 WINDOW_X_minus7 = 0xFF4B;
constexpr u16 BG_PALETTE_DATA = 0xFF47;
constexpr u16 SPRITE_PALETTE_0_DATA = 0xFF48;
constexpr u16 SPRITE_PALETTE_1_DATA = 0xFF49;
constexpr u16 DMA_TRANSFER = 0xFF46;

// FF40 LCD Control Flags (as bitmasks)
constexpr u8 LCD_DISPLAY_ENABLE = 0x80;
constexpr u8 WINDOW_TILE_MAP_SELECT = 0x40;
constexpr u8 WINDOW_DISPLAY_ENABLE = 0x20;
constexpr u8 BG_AND_WINDOW_TILE_SELECT = 0x10;
constexpr u8 BG_TILE_MAP_SELECT = 0x08;
constexpr u8 SPRITE_SIZE = 0x04;
constexpr u8 SPRITE_DISPLAY_ENABLE = 0x02;
constexpr u8 BG_DISPLAY = 0x01;

// FF41 LCDC Status Flags (as bitmasks)
constexpr u8 LYC_LY_COINCIDENCE_INTERRUPT = 0x40;
constexpr u8 MODE_2_OAM_INTERRUPT = 0x20;
constexpr u8 MODE_1_VBLANK_INTERRUPT = 0x10;
constexpr u8 MODE_0_HBLANK_INTERRUPT = 0x08;
constexpr u8 COINCIDENCE_FLAG = 0x04;

class GPU {
public:
    bool hitVBlank;
public:
    GPU(CPU* cpu, MMU* mmu);
    void reset();
    void tick(u32 ticks);
    u8* getDisplayState();
    u8* getBackgroundState();
    u8* getTileData();
    u8* getVRAM();
    u8* getOAM();

    u8 readByte(u16 address);
    void writeByte(u16 address, u8 value);
    u16 readWord(u16 address);
    void writeWord(u16 address, u16 value);
private:
    CPU* cpu;
    MMU* mmu;

    GPU_MODE mode;
    int modeclock;
    int DMATicks;

    std::vector<u8> displayState;
    std::vector<u8> backgroundState;
    std::vector<std::vector<u8>> background;
    //std::vector<std::vector<u8>> backgroundTmp;
    std::vector<u8> VRAM;
    std::vector<u8> OAM;
    std::vector<u8> tileData;
private:
    u8 getReg(u16 regAddress);
    void setReg(u16 regAddress, u8 value);
    u8 getMode();
    void setMode(GPU_MODE mode);

    void renderScanline();
    void renderBGScanline(u8 yCoord);
    void renderWindowScanline();
    void renderSpriteScanline();

    void setBGColor(u8 color);
};

#endif //PHOS_GPU_H
