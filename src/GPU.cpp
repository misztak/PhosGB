#include "GPU.h"
#include "CPU.h"

GPU::GPU(CPU* c, MMU* m):
    cpu(c),
    mmu(m),
    mode(VBLANK),
    modeclock(0),
    displayState(DISPLAY_TEXTURE_SIZE, WHITE),
    background(256, std::vector<u8>(256, WHITE)),
    backgroundTmp(144, std::vector<u8>(160, WHITE)),
    VRAM(VRAM_SIZE),
    OAM(OAM_SIZE) {}

void GPU::reset() {
    setReg(LCDC_Y_COORDINATE, 153);
    setReg(SCROLL_Y, 0x00);
    setReg(SCROLL_X, 0x00);
    setReg(LY_COMPARE, 0x00);
    setReg(BG_PALETTE_DATA, 0xFC);
    setReg(SPRITE_PALETTE_0_DATA, 0xFF);
    setReg(SPRITE_PALETTE_1_DATA, 0xFF);
    setReg(WINDOW_Y, 0x00);
    setReg(WINDOW_X_minus7, 0x00);

}

void GPU::tick(u32 ticks) {
    modeclock += ticks;
    u8* line = &mmu->mappedIO[LCDC_Y_COORDINATE - 0xFF00];
    switch (mode) {
        case READ_OAM:
            if (modeclock >= 80) {
                // enter scanline mode 3 (READ_BOTH)
                modeclock = 0;
                setMode(READ_BOTH);
            }
            break;
        case READ_BOTH:
            if (modeclock >= 172) {
                // beginning of HBLANK
                modeclock = 0;
                setMode(HBLANK);

                renderScanline();
                if (isBitSet(getReg(LCDC_STATUS), MODE_0_HBLANK_INTERRUPT)) {
                    cpu->requestInterrupt(INTERRUPT_LCD_STAT);
                }
            }
            break;
        case HBLANK:
            if (modeclock >= 204) {
                modeclock = 0;
                (*line)++;
                if ((*line) == 144) {
                    // beginning of VBLANK
                    setMode(VBLANK);
                    hitVBlank = true;

                     cpu->requestInterrupt(INTERRUPT_VBLANK);
                     if (isBitSet(getReg(LCDC_STATUS), MODE_1_VBLANK_INTERRUPT)) {
                         cpu->requestInterrupt(INTERRUPT_LCD_STAT);
                     }
                } else {
                    setMode(READ_OAM);
                    if (isBitSet(getReg(LCDC_STATUS), MODE_2_OAM_INTERRUPT)) {
                        cpu->requestInterrupt(INTERRUPT_LCD_STAT);
                    }
                }
            }
            break;
        case VBLANK:
            if (modeclock >= 456) {
                modeclock = 0;
                (*line)++;
                if ((*line) > 153) {
                    setMode(READ_OAM);
                    (*line) = 0;
                    if (isBitSet(getReg(LCDC_STATUS), MODE_2_OAM_INTERRUPT)) {
                        cpu->requestInterrupt(INTERRUPT_LCD_STAT);
                    }
                }
            }
            break;
    }
    // TODO: coincidence flag?
}

void GPU::renderScanline() {
    renderBGScanline();

    if (isBitSet(getReg(LCD_CONTROL), WINDOW_DISPLAY_ENABLE)) {
        printf("Window enabled");
        renderWindowScanline();
    }

    // TODO: copy SINGLE LINE into displayState

    if (isBitSet(getReg(LCD_CONTROL), SPRITE_DISPLAY_ENABLE)) {
        renderSpriteScanline();
    }
}

void GPU::renderBGScanline() {
    if (!isBitSet(getReg(LCD_CONTROL), BG_DISPLAY)) {
        setBGColor(WHITE);
        return;
    }

    int paletteData = getReg(BG_PALETTE_DATA);
    const u8 palette[] {
        colors[paletteData & 0x03],
        colors[(paletteData >> 2) & 0x03],
        colors[(paletteData >> 4) & 0x03],
        colors[(paletteData >> 6) & 0x03]
    };

    u16 tileNumberMap = isBitSet(getReg(LCD_CONTROL), BG_TILE_MAP_SELECT) ? (u16) 0x9C00 : (u16) 0x9800;
    tileNumberMap -= 0x8000;

    u16 tileData = isBitSet(getReg(LCD_CONTROL), BG_AND_WINDOW_TILE_SELECT) ? (u16) 0x8000 : (u16) 0x9000;
    tileData -= 0x8000;

    u8 tileY = (u8)(((getReg(LCDC_Y_COORDINATE) + getReg(SCROLL_Y)) / 8) % 32);
    u8 tileYOffset = (u8)((getReg(LCDC_Y_COORDINATE) + getReg(SCROLL_Y)) % 8);

    for (u8 x=0; x<160; x++) {
        u8 tileX = (u8)(((getReg(SCROLL_X) + x) / 8) % 32);
        u8 tileNumber = VRAM[(u16)(tileNumberMap + (tileY * 32) + tileX)];

        u16 tileDataPtr = 0;
        if (isBitSet(getReg(LCD_CONTROL), BG_AND_WINDOW_TILE_SELECT)) {
            tileDataPtr = (u16)(tileData + tileNumber * 0x10);
        } else {
            tileDataPtr = (u16)(tileData + static_cast<char>(tileNumber) * 0x10);
        }

        tileDataPtr += (u16)(tileYOffset * 2);
        u8 b1 = VRAM[tileDataPtr];
        u8 b2 = VRAM[tileDataPtr + 1];

        u8 bit = (u8)(7 - ((getReg(SCROLL_X) + x) % 8));
        if (bit > 7) {
            printf("Invalid value for bitmask %d\n", bit);
        }
        u8 bitMask = 1 << bit;
        u8 pLo = isBitSet(b1, bitMask) ? 1 : 0;
        u8 pHi = isBitSet(b2, bitMask) ? 2 : 0;

        u8 color = palette[pLo + pHi];

        //int index = ((getReg(LCDC_Y_COORDINATE) * 160) + x) * 4;
        backgroundTmp[getReg(LCDC_Y_COORDINATE)][x] = color;
    }
}

void GPU::renderWindowScanline() {
    int winY = getReg(LCDC_Y_COORDINATE) - getReg(WINDOW_Y);
    if (winY < 0) {
        return;
    }

    const u8 palette[] {
        colors[getReg(BG_PALETTE_DATA) & 0x03],
        colors[(getReg(BG_PALETTE_DATA) >> 2) & 0x03],
        colors[(getReg(BG_PALETTE_DATA) >> 4) & 0x03],
        colors[(getReg(BG_PALETTE_DATA) >> 6) & 0x03]
    };

    u16 tileNumberMap = isBitSet(getReg(LCD_CONTROL), WINDOW_TILE_MAP_SELECT) ? (u16) 0x9C00 : (u16) 0x9800;
    tileNumberMap -= 0x8000;

    u16 tileData = isBitSet(getReg(LCD_CONTROL), BG_AND_WINDOW_TILE_SELECT) ? (u16) 0x8000 : (u16) 0x9000;
    tileData -= 0x8000;

    u8 tileY = (u8)(winY / 8);
    u8 tileYOffset = (u8)(winY % 8);

    int winX = getReg(WINDOW_X_minus7) - 7;
    for (int x=0; x<160; x++) {
        if (x < winX) {
            continue;
        }

        u8 tileX = (u8)((x - winX) / 8);
        u8 tileNumber = VRAM[tileNumberMap + (tileY * 32) + tileX];
        u16 tileDataPtr = 0;

        if (isBitSet(getReg(LCD_CONTROL), BG_AND_WINDOW_TILE_SELECT)) {
            tileDataPtr = (u16)(tileData + tileNumber * 0x10);
        } else {
            tileDataPtr = (u16)(tileData + static_cast<char>(tileNumber) * 0x10);
        }
        tileDataPtr += (u16)(tileYOffset * 2);

        u8 b1 = VRAM[tileDataPtr];
        u8 b2 = VRAM[tileDataPtr + 1];

        u8 bit = (u8)(7 - x % 8);
        if (bit > 7) {
            printf("Invalid value for bitmask %d\n", bit);
        }
        u8 bitMask = 1 << bit;
        u8 pLo = isBitSet(b1, bitMask) ? 1 : 0;
        u8 pHi = isBitSet(b2, bitMask) ? 2 : 0;
        u8 color = palette[pLo + pHi];

        //int index = ((getReg(LCDC_Y_COORDINATE) * 160) + x) * 4;
        backgroundTmp[getReg(LCDC_Y_COORDINATE)][x] = color;
    }
}

void GPU::renderSpriteScanline() {
    printf("Sprites not yet implemented\n");
}

u8 GPU::readByte(u16 address) {
    if (address <= 0x9FFF) {
        return VRAM[address - 0x8000];
    } else {
        return OAM[address - 0xFE00];
    }
}

void GPU::writeByte(u16 address, u8 value) {
    if (address <= 0x9FFF) {
        VRAM[address - 0x8000] = value;
    } else {
        OAM[address - 0xFE00] = value;
    }
}

u16 GPU::readWord(u16 address) {
    return readByte(address) | (readByte(address + 1) << 8);
}

void GPU::writeWord(u16 address, u16 value) {
    u8 low = value & 0xFF;
    u8 high = (value >> 8) & 0xFF;
    writeByte(address, low);
    writeByte(address + 1, high);
}

u8 GPU::getReg(u16 regAddress) {
    return mmu->readByte(regAddress);
}

void GPU::setReg(u16 regAddress, u8 value) {
    mmu->writeByte(regAddress, value);
}

u8 GPU::getMode() {
    return getReg(LCDC_STATUS) & (u8) 0x03;
}

void GPU::setMode(GPU_MODE newMode) {
    u8 oldValue = mmu->mappedIO[LCDC_STATUS - 0xFF00];
    mmu->mappedIO[LCDC_STATUS - 0xFF00] = (oldValue & ~((u8) 0x03)) | newMode;
    //setReg(LCDC_STATUS, (getReg(LCDC_STATUS) & ~((u8) 0x03)) | newMode);
    mode = newMode;
}

u8* GPU::getDisplayState() {
    int counter = 0;
    for (u32 i=0; i<DISPLAY_TEXTURE_SIZE; i+=4) {
        int x = counter % WIDTH;
        int y = counter / WIDTH;
        counter++;
        displayState[i] = backgroundTmp[y][x];
        displayState[i+1] = backgroundTmp[y][x];
        displayState[i+2] = backgroundTmp[y][x];
        displayState[i+3] = 0xFF;
    }
    return displayState.data();
}

void GPU::setBGColor(COLORS color) {
    for (int y=0; y<256; y++) {
        for (int x=0; x<256; x++) {
            background[y][x] = color;
        }
    }
}

u8 *GPU::getVRAM() {
    return VRAM.data();
}

u8 *GPU::getOAM() {
    return OAM.data();
}
