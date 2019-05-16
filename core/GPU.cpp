#include "GPU.h"
#include "CPU.h"

GPU::GPU(CPU* c, MMU* m) :
    hitVBlank(false),
    modeclock(0),
    DMATicks(0),
    cpu(c),
    mmu(m),
    mode(VBLANK),
    displayState(DISPLAY_TEXTURE_SIZE, 255),
    backgroundState(262144, 255),
    background(256, std::vector<u8>(256, 255)),
    //backgroundTmp(144, std::vector<u8>(160, 255)),
    tileData(6144 * 4 * 8, 255) {}

void GPU::reset() {
    hitVBlank = false;
    setMode(VBLANK);
    modeclock = 0;
    DMATicks = 0;
    std::fill(displayState.begin(), displayState.end(), 255);
    std::fill(backgroundState.begin(), backgroundState.end(), 255);
    for (std::vector<u8>& v : background) std::fill(v.begin(), v.end(), 255);
    std::fill(tileData.begin(), tileData.end(), 255);

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
    //if (DMATicks > 0) DMATicks -= 0;

    modeclock += ticks;
    u8 line = getReg(LCDC_Y_COORDINATE);
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
                line++;
                if (line == 144) {
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
                line++;
                if (line > 153) {
                    setMode(READ_OAM);
                    line = 0;
                    if (isBitSet(getReg(LCDC_STATUS), MODE_2_OAM_INTERRUPT)) {
                        cpu->requestInterrupt(INTERRUPT_LCD_STAT);
                    }
                }
            }
            break;
    }

    if (getReg(LY_COMPARE) == getReg(LCDC_Y_COORDINATE)) {
        setReg(LCDC_STATUS, setBit(getReg(LCDC_STATUS), COINCIDENCE_FLAG));
        if (isBitSet(getReg(LCDC_STATUS), LYC_LY_COINCIDENCE_INTERRUPT)) {
            cpu->requestInterrupt(INTERRUPT_LCD_STAT);
        }
    } else {
        setReg(LCDC_STATUS, clearBit(getReg(LCDC_STATUS), COINCIDENCE_FLAG));
    }

    setReg(LCDC_Y_COORDINATE, line);
}

void GPU::renderScanline() {
    renderBGScanline(getReg(LCDC_Y_COORDINATE));

    if (isBitSet(getReg(LCD_CONTROL), WINDOW_DISPLAY_ENABLE)) {
        renderWindowScanline();
    }

    int y = getReg(LCDC_Y_COORDINATE);
    int index = y * 160 * 4;
    for (int x=0; x<160; x++) {
        displayState[index] = background[y][x];
        displayState[index + 1] = background[y][x];
        displayState[index + 2] = background[y][x];
        index += 4;
    }

    if (isBitSet(getReg(LCD_CONTROL), SPRITE_DISPLAY_ENABLE)) {
        renderSpriteScanline();
    }
}

void GPU::renderBGScanline(u8 yCoord) {
    if (!isBitSet(getReg(LCD_CONTROL), BG_DISPLAY)) {
        setBGColor(0xFF);
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

    u8 tileY = (u8)(((yCoord + getReg(SCROLL_Y)) / 8) % 32);
    u8 tileYOffset = (u8)((yCoord + getReg(SCROLL_Y)) % 8);

    for (u8 x=0; x<255; x++) {
        u8 tileX = (u8)(((getReg(SCROLL_X) + x) / 8) % 32);
        u8 tileNumber = mmu->VRAM[(u16)(tileNumberMap + (tileY * 32) + tileX)];

        u16 tileDataPtr = 0;
        if (isBitSet(getReg(LCD_CONTROL), BG_AND_WINDOW_TILE_SELECT)) {
            tileDataPtr = (u16)(tileData + tileNumber * 0x10);
        } else {
            tileDataPtr = (u16)(tileData + static_cast<char>(tileNumber) * 0x10);
        }

        tileDataPtr += (u16)(tileYOffset * 2);
        u8 b1 = mmu->VRAM[tileDataPtr];
        u8 b2 = mmu->VRAM[tileDataPtr + 1];

        u8 bit = (u8)(7 - ((getReg(SCROLL_X) + x) % 8));
        if (bit > 7) {
            printf("Invalid value for bitmask %d\n", bit);
        }
        u8 bitMask = 1 << bit;
        u8 pLo = isBitSet(b1, bitMask) ? 1 : 0;
        u8 pHi = isBitSet(b2, bitMask) ? 2 : 0;

        u8 color = palette[pLo + pHi];

        //int index = ((getReg(LCDC_Y_COORDINATE) * 160) + x) * 4;
        background[yCoord][x] = color;
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
        u8 tileNumber = mmu->VRAM[tileNumberMap + (tileY * 32) + tileX];
        u16 tileDataPtr = 0;

        if (isBitSet(getReg(LCD_CONTROL), BG_AND_WINDOW_TILE_SELECT)) {
            tileDataPtr = (u16)(tileData + tileNumber * 0x10);
        } else {
            tileDataPtr = (u16)(tileData + static_cast<char>(tileNumber) * 0x10);
        }
        tileDataPtr += (u16)(tileYOffset * 2);

        u8 b1 = mmu->VRAM[tileDataPtr];
        u8 b2 = mmu->VRAM[tileDataPtr + 1];

        u8 bit = (u8)(7 - x % 8);
        if (bit > 7) {
            printf("Invalid value for bitmask %d\n", bit);
        }
        u8 bitMask = 1 << bit;
        u8 pLo = isBitSet(b1, bitMask) ? 1 : 0;
        u8 pHi = isBitSet(b2, bitMask) ? 2 : 0;
        u8 color = palette[pLo + pHi];

        //int index = ((getReg(LCDC_Y_COORDINATE) * 160) + x) * 4;
        background[getReg(LCDC_Y_COORDINATE)][x] = color;
    }
}

void GPU::renderSpriteScanline() {
    const u8 SPRITE_SIZE_BYTES = 16;
    const u8 bgPalette[] {
        colors[getReg(BG_PALETTE_DATA) & 0x03],
        colors[getReg(BG_PALETTE_DATA >> 2) & 0x03],
        colors[getReg(BG_PALETTE_DATA >> 4) & 0x03],
        colors[getReg(BG_PALETTE_DATA >> 6) & 0x03],
    };

    for (int i=156; i>=0; i-=4) {
        u8 objY = mmu->OAM[i];
        u8 spriteSize = isBitSet(getReg(LCD_CONTROL), SPRITE_SIZE) ? 0x10 : 0x08;
        int height = spriteSize;

        int y = objY - 16;
        if ((y <= getReg(LCDC_Y_COORDINATE)) && ((y + height) > getReg(LCDC_Y_COORDINATE))) {
            u8 objX = mmu->OAM[i + 1];
            u8 spriteTileNumber = mmu->OAM[i + 2];
            u8 spriteFlags = mmu->OAM[i + 3];

            if (spriteSize == 0x10) {
                spriteTileNumber &= 0xFE;
            }

            u8 paletteNumber = isBitSet(spriteFlags, 0x10) ? 0x01 : 0x00;
            int x = objX - 8;

            const u16 tileData = 0x0000;
            u8 palette[] {
                0x00,
                colors[(paletteNumber == 0x00) ? (getReg(SPRITE_PALETTE_0_DATA) >> 2 & 0x03) : (getReg(SPRITE_PALETTE_1_DATA) >> 2 & 0x03)],
                colors[(paletteNumber == 0x00) ? (getReg(SPRITE_PALETTE_0_DATA) >> 4 & 0x03) : (getReg(SPRITE_PALETTE_1_DATA) >> 4 & 0x03)],
                colors[(paletteNumber == 0x00) ? (getReg(SPRITE_PALETTE_0_DATA) >> 6 & 0x03) : (getReg(SPRITE_PALETTE_1_DATA) >> 6 & 0x03)],
            };

            u16 tilePtr = tileData + (spriteTileNumber * SPRITE_SIZE_BYTES);
            u8 tileYOffset = isBitSet(spriteFlags, 0x40) ? ((height - 1) - (getReg(LCDC_Y_COORDINATE) - y)) : (getReg(LCDC_Y_COORDINATE) - y);
            tilePtr += (tileYOffset * 2);

            u8 low = mmu->VRAM[tilePtr];
            u8 high = mmu->VRAM[(u16)tilePtr + 1];

            for (int indexX=0; indexX<8; indexX++) {
                int pixelX = x + indexX;
                if (pixelX >= 0 && pixelX < 160) {
                    u8 bit = isBitSet(spriteFlags, 0x20) ? indexX : 7 - indexX;
                    u8 bitMask = 0x01 << bit;
                    u8 pixelVal = 0x00;
                    if (isBitSet(high, bitMask)) pixelVal |= 2;
                    if (isBitSet(low, bitMask)) pixelVal |= 1;
                    u8 color = palette[pixelVal];

                    if (pixelVal != 0x00) {
                        int index = ((getReg(LCDC_Y_COORDINATE) * 160) + pixelX) * 4;
                        if (!isBitSet(spriteFlags, 0x80) || (background[getReg(LCDC_Y_COORDINATE)][pixelX] == bgPalette[0x00])) {
                            //backgroundTmp[getReg(LCDC_Y_COORDINATE)][pixelX] = color;
                            displayState[index] = color;
                            displayState[index + 1] = color;
                            displayState[index + 2] = color;
                        }
                    }
                }
            }
        }

    }
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
    u8 oldValue = mmu->IO[LCDC_STATUS - 0xFF00];
    mmu->IO[LCDC_STATUS - 0xFF00] = (oldValue & ~((u8) 0x03)) | newMode;
    //setReg(LCDC_STATUS, (getReg(LCDC_STATUS) & ~((u8) 0x03)) | newMode);
    mode = newMode;
}

u8* GPU::getDisplayState() {
    return displayState.data();
}

u8* GPU::getBackgroundState() {
    int counter = 0;
    for (int i=144; i<256; i++) {
        renderBGScanline(i);
    }
    for (int y=0; y<256; y++) {
        for (int x=0; x<256; x++) {
            backgroundState[counter] = background[y][x];
            backgroundState[counter + 1] = background[y][x];
            backgroundState[counter + 2] = background[y][x];
            counter += 4;
        }
    }
    return backgroundState.data();
}

u8* GPU::getTileData() {
    int counter = 0;
    for (int i=0; i<6144; i++) {
        for (int b=7; b>=0; b--) {
            u8 color = ((mmu->VRAM[i] >> b) & 0x01) ? 255 : 0;
            tileData[counter] = color;
            tileData[counter + 1] = color;
            tileData[counter + 2] = color;
            counter += 4;
        }
    }
    return tileData.data();
}

void GPU::setBGColor(u8 color) {
    for (int y=0; y<256; y++) {
        for (int x=0; x<256; x++) {
            background[y][x] = color;
        }
    }
}

