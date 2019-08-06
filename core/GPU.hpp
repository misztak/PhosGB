#ifndef PHOS_GPU_HPP
#define PHOS_GPU_HPP

#include <array>

#include "Common.hpp"
#include "MMU.hpp"

class CPU;

enum GPU_MODE { HBLANK, VBLANK, READ_OAM, READ_BOTH };

const u8 colors[] { 255, 192, 96, 0 };

// GPU Registers
constexpr u16 LCD_CONTROL           = 0xFF40;
constexpr u16 LCDC_STATUS           = 0xFF41;
constexpr u16 SCROLL_Y              = 0xFF42;
constexpr u16 SCROLL_X              = 0xFF43;
constexpr u16 LCDC_Y_COORDINATE     = 0xFF44;
constexpr u16 LY_COMPARE            = 0xFF45;
constexpr u16 WINDOW_Y              = 0xFF4A;
constexpr u16 WINDOW_X_minus7       = 0xFF4B;
constexpr u16 BG_PALETTE_DATA       = 0xFF47;
constexpr u16 SPRITE_PALETTE_0_DATA = 0xFF48;
constexpr u16 SPRITE_PALETTE_1_DATA = 0xFF49;
constexpr u16 DMA_TRANSFER          = 0xFF46;

// FF40 LCD Control Flags (bit position)
constexpr u8 LCD_DISPLAY_ENABLE         = 7;
constexpr u8 WINDOW_TILE_MAP_SELECT     = 6;
constexpr u8 WINDOW_DISPLAY_ENABLE      = 5;
constexpr u8 BG_AND_WINDOW_TILE_SELECT  = 4;
constexpr u8 BG_TILE_MAP_SELECT         = 3;
constexpr u8 SPRITE_SIZE                = 2;
constexpr u8 SPRITE_DISPLAY_ENABLE      = 1;
constexpr u8 BG_DISPLAY                 = 0;

// FF41 LCDC Status Flags (bit position)
constexpr u8 LYC_LY_COINCIDENCE_INTERRUPT   = 6;
constexpr u8 MODE_2_OAM_INTERRUPT           = 5;
constexpr u8 MODE_1_VBLANK_INTERRUPT        = 4;
constexpr u8 MODE_0_HBLANK_INTERRUPT        = 3;
constexpr u8 COINCIDENCE_FLAG               = 2;

struct Pixel {
    u8 type;
    u8 palette;
    u16 color;
    u8 r, g, b;
    Pixel() : type(0), palette(0), color(0x7FFF), r(0xFF), g(0xFF), b(0xFF) {};
    void setColor(u8 red, u8 green, u8 blue) { r = red, g = green, b = blue; }
    void clear() { type = 0, palette = 0, color = 0x7FFF; setColor(0xFF, 0xFF, 0xFF); }
};

class GPU {
public:
    bool hitVBlank;
    int modeclock;
    int DMATicks;

    bool useCustomPalette;
    bool showViewportBorder;

    std::map<u8, std::array<u8, 3>> customPalette;
public:
    GPU(CPU* cpu, MMU* mmu);
    void reset();
    void tick(u32 ticks);
    u8* getDisplayState();
    u8* getBackgroundState();
    u8* getTileData(int offset);

    u8 getMode();
    void setMode(GPU_MODE mode);

    void setBGColor(u8 color);
    void colorCorrect(u16 original, u8& r, u8& g, u8& b);

    void serialize(serializer& s);
private:
    CPU* cpu;
    MMU* mmu;

    GPU_MODE mode;
    u32 wyc;

    std::vector<Pixel> pixelLine;
    std::vector<u8> displayState;
    std::vector<u8> backgroundState;
    std::vector<std::vector<u8>> background;
    std::vector<u8> tileData;
private:
    u8 getReg(u16 regAddress);
    void setReg(u16 regAddress, u8 value);

    void renderScanline();
    void fetchTileData(bool mapSelect, u8 posY, u8 posX, u16& tile, u16& attribute, u16& data);
    void renderBGScanline(bool fullLine = false, u8 yCoord = 0);
    void renderWindowScanline();
    void renderSpriteScanline();
    unsigned hflip(unsigned data);
    u16 getColor(u8 type, u16 attribute, u16 paletteIndex);
};

#endif //PHOS_GPU_HPP
