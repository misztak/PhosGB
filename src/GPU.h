#ifndef PHOS_GPU_H
#define PHOS_GPU_H

#include "Common.h"
#include "MMU.h"

class CPU;

enum GPU_MODE { HBLANK, VBLANK, READ_OAM, READ_BOTH };

enum COLORS { WHITE = 255, LIGHT_GREY = 192, DARK_GREY = 96, BLACK = 0 };

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

class GPU {
public:
    bool hitVBlank;
public:
    GPU(CPU* cpu, MMU* mmu);
    void reset();
    void tick(u32 ticks);
    u8* getDisplayState();

    u8 readByte(u16 address);
    void writeByte(u16 address, u8 value);
    u16 readWord(u16 address);
    void writeWord(u16 address, u16 value);
private:
    CPU* cpu;
    MMU* mmu;

    GPU_MODE mode;
    int modeclock;
    int line;

    u8 scrollX;
    u8 scrollY;

    std::vector<u8> displayState;
    std::vector<std::vector<u8>> background;
    std::vector<u8> VRAM;
    std::vector<u8> OAM;
private:
    u8 getReg(u16 regAddress);
    void setReg(u16 regAddress, u8 value);
};

#endif //PHOS_GPU_H
