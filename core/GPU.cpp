#include "GPU.hpp"
#include "CPU.hpp"

GPU::GPU(CPU* c, MMU* m) :
    hitVBlank(false),
    modeclock(0),
    DMATicks(0),
    useCustomPalette(false),
    showViewportBorder(true),
    cpu(c),
    mmu(m),
    mode(VBLANK),
    wyc(0),
    pixelLine(160),
    displayState(DISPLAY_TEXTURE_SIZE, 255),
    backgroundState(262144, 255),
    background(256, std::vector<u8>(256, 255)),
    tileData(64 * 4) {
    // map customizable RGB values to each palette value
    customPalette[colors[0]] = {224, 248, 208};
    customPalette[colors[1]] = {136, 192, 112};
    customPalette[colors[2]] = { 52, 104,  86};
    customPalette[colors[3]] = {  8,  24,  32};
}

void GPU::reset() {
    hitVBlank = false;
    setMode(VBLANK);
    modeclock = 0;
    DMATicks = 0;
    wyc = 0;
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
    if (cpu->doubleSpeedMode) ticks /= 2;
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
                    if (cpu->gbMode == CGB && mmu->VramDma.enabled)
                        mmu->performHDMA();

                    setMode(READ_OAM);
                    if (isBitSet(getReg(LCDC_STATUS), MODE_2_OAM_INTERRUPT)) {
                        cpu->requestInterrupt(INTERRUPT_LCD_STAT);
                    }
                }
            }
            break;
        case VBLANK:
            if (modeclock >= 456) {
                modeclock = 0; wyc = 0;
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

    if (getReg(LY_COMPARE) == line) {
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
    for (int p=0; p<160; p++) pixelLine[p].clear();

    // white background
    if (!isBitSet(getReg(LCD_CONTROL), BG_DISPLAY) && cpu->gbMode == DMG) {
        setBGColor(0xFF);
    } else {
        renderBGScanline();
    }

    if (isBitSet(getReg(LCD_CONTROL), WINDOW_DISPLAY_ENABLE)) {
        renderWindowScanline();
    }

    int y = getReg(LCDC_Y_COORDINATE);
    int index = y * 160 * 4;
    for (int x=0; x<160; x++) {
        if (cpu->gbMode == DMG) {
            if (useCustomPalette) {
                displayState[index] = customPalette[pixelLine[x].r][0];
                displayState[index + 1] = customPalette[pixelLine[x].g][1];
                displayState[index + 2] = customPalette[pixelLine[x].b][2];
                index += 4;
            } else {
                displayState[index] = pixelLine[x].r;
                displayState[index + 1] = pixelLine[x].g;
                displayState[index + 2] = pixelLine[x].b;
                index += 4;
            }
        } else {
            u8 r, g, b;
            colorCorrect(pixelLine[x].color, r, g, b);
            displayState[index    ] = r;
            displayState[index + 1] = g;
            displayState[index + 2] = b;
            index += 4;
        }
    }

    if (isBitSet(getReg(LCD_CONTROL), SPRITE_DISPLAY_ENABLE)) {
        renderSpriteScanline();
    }
}

void GPU::fetchTileData(bool mapSelect, u8 posY, u8 posX, u16& tile, u16& attribute, u16& data) {
    // get the start of the selected map and add the offset to the selected tile
    u16 tileAddress = mapSelect ? 0x1C00u : 0x1800u;
    tileAddress += (((posY / 8) * 32) + (posX / 8)) % 1024;

    u16 tileData = 0x0000;
    u8 yOffset = posY & 0x07;

    tile = mmu->VRAM[tileAddress];
    if (cpu->gbMode == CGB) {
        attribute = mmu->VRAM[VRAM_BANK_SIZE + tileAddress];
        // select correct bank
        if (attribute & 0x08) tileData += VRAM_BANK_SIZE;
        // check for up/down flip
        if (attribute & 0x40) yOffset ^= 7;
    }

    if (isBitSet(getReg(LCD_CONTROL), BG_AND_WINDOW_TILE_SELECT)) {
        tileData += tile * 0x10;
    } else {
        tileData += 0x1000 + static_cast<char>(tile) * 0x10;
    }
    tileData += yOffset * 2;

    data = mmu->VRAM[tileData++];
    data |= mmu->VRAM[tileData] << 8;

    // check for left/right flip
    if (cpu->gbMode == CGB && attribute & 0x20) data = hflip(data);
}

void GPU::renderBGScanline(bool fullLine, u8 yCoord) {
    // DMG palette (should probably move this somewhere else)
    int paletteData = getReg(BG_PALETTE_DATA);
    const u8 palette[4] {
            colors[paletteData & 0x03],
            colors[(paletteData >> 2) & 0x03],
            colors[(paletteData >> 4) & 0x03],
            colors[(paletteData >> 6) & 0x03],
    };

    u8 posY = getReg(LCDC_Y_COORDINATE) + getReg(SCROLL_Y);
    u8 posX = getReg(SCROLL_X);
    unsigned lineWidth = 160;
    if (fullLine) {
        posY = yCoord;
        posX = 0;
        lineWidth = 256;
    }

    u16 tile = 0, attribute = 0, data = 0;
    bool mapSelect = isBitSet(getReg(LCD_CONTROL), BG_TILE_MAP_SELECT);
    fetchTileData(mapSelect, posY, posX, tile, attribute, data);

    // draw one line
    u8 bit = posX % 8;
    for (unsigned x=0; x<lineWidth; x++) {
        u8 pLo = ((data & (0x0080 >> bit)) ? 1 : 0);
        u8 pHi = ((data & (0x8000 >> bit)) ? 2 : 0);
        u8 paletteIndex = pLo + pHi;

        pixelLine[x].type = (attribute & 0x80) ? 3 : 1;
        pixelLine[x].palette = paletteIndex;
        if (cpu->gbMode == DMG) {
            u8 color = palette[paletteIndex];
            pixelLine[x].setColor(color, color, color);
        } else {
            u16 color = getColor(0, attribute, paletteIndex);
            pixelLine[x].color = color;
            pixelLine[x].setColor(color & 0x001F, (color & 0x03E0) >> 5, (color & 0x7C00) >> 10);
        }

        posX++;
        bit = (bit + 1) % 8;
        if (bit == 0) fetchTileData(mapSelect, posY, posX, tile, attribute, data);
    }
}

void GPU::renderWindowScanline() {
    // DMG palette (should probably move this somewhere else)
    int paletteData = getReg(BG_PALETTE_DATA);
    const u8 palette[4] {
            colors[paletteData & 0x03],
            colors[(paletteData >> 2) & 0x03],
            colors[(paletteData >> 4) & 0x03],
            colors[(paletteData >> 6) & 0x03],
    };

    if (getReg(WINDOW_X_minus7) >= 167u) return;
    if ((unsigned)(getReg(LCDC_Y_COORDINATE) - getReg(WINDOW_Y)) >= 144u) return;
    u8 posX = (7 - getReg(WINDOW_X_minus7)) & 0xFF;
    u8 posY = wyc++;

    u16 tile = 0, attribute = 0, data = 0;
    bool mapSelect = isBitSet(getReg(LCD_CONTROL), WINDOW_TILE_MAP_SELECT);
    fetchTileData(mapSelect, posY, posX, tile, attribute, data);

    // draw one line
    u8 bit = posX % 8;
    for (unsigned x=0; x<160; x++) {
        u8 pLo = ((data & (0x0080 >> bit)) ? 1 : 0);
        u8 pHi = ((data & (0x8000 >> bit)) ? 2 : 0);
        u8 paletteIndex = pLo + pHi;

        if (x - (getReg(WINDOW_X_minus7) - 7) <= 160u) {
            pixelLine[x].type = (attribute & 0x80) ? 3 : 1;
            pixelLine[x].palette = paletteIndex;
            if (cpu->gbMode == DMG) {
                u8 color = palette[paletteIndex];
                pixelLine[x].setColor(color, color, color);
            } else {
                u16 color = getColor(0, attribute, paletteIndex);
                pixelLine[x].color = color;
                pixelLine[x].setColor(color & 0x001F, (color & 0x03E0) >> 5, (color & 0x7C00) >> 10);
            }
        }

        posX++;
        bit = (bit + 1) % 8;
        if (bit == 0) fetchTileData(mapSelect, posY, posX, tile, attribute, data);
    }
}

void GPU::renderSpriteScanline() {
    const u8 spriteSize = isBitSet(getReg(LCD_CONTROL), SPRITE_SIZE) ? 16 : 8;
    unsigned sprites[10] = {};
    unsigned spriteCount = 0;

    for (unsigned s=0; s<40; s++) {
        unsigned sy = mmu->OAM[s * 4] - 16;

        sy = getReg(LCDC_Y_COORDINATE) - sy;
        if (sy >= spriteSize) continue;
        sprites[spriteCount++] = s;
        if (spriteCount == 10) break;
    }

    // sort sprites by x-coordinate
    for (unsigned s1=0; s1<spriteCount; s1++) {
        for (unsigned s2=s1+1; s2<spriteCount; s2++) {
            int sx1 = mmu->OAM[sprites[s1] * 4 + 1] - 8;
            int sx2 = mmu->OAM[sprites[s2] * 4 + 1] - 8;
            if (sx2 < sx1) {
                unsigned tmp = sprites[s1];
                sprites[s1] = sprites[s2];
                sprites[s2] = tmp;
            }
        }
    }

    // render backwards
    for (int s=spriteCount-1; s>=0; s--) {
        unsigned spriteAddress = sprites[s] * 4;
        unsigned spriteY    = mmu->OAM[spriteAddress] - 16;
        unsigned spriteX    = mmu->OAM[spriteAddress + 1] - 8;
        unsigned spriteTile = mmu->OAM[spriteAddress + 2];
        unsigned spriteAttr = mmu->OAM[spriteAddress + 3];
        if (spriteSize == 16) spriteTile &= 0xFE;

        u8 relY = getReg(LCDC_Y_COORDINATE) - spriteY;
        if (relY >= spriteSize) continue;
        // vertical flip
        if (spriteAttr & 0x40) relY = (spriteSize - 1) - relY;

        u8 dmgPaletteNumber = isBitSet(spriteAttr, 0x10);
        const u8 dmgPalette[4] {
            0x00,
            colors[dmgPaletteNumber ? (getReg(SPRITE_PALETTE_1_DATA) >> 2 & 0x03) : (getReg(SPRITE_PALETTE_0_DATA) >> 2 & 0x03)],
            colors[dmgPaletteNumber ? (getReg(SPRITE_PALETTE_1_DATA) >> 4 & 0x03) : (getReg(SPRITE_PALETTE_0_DATA) >> 4 & 0x03)],
            colors[dmgPaletteNumber ? (getReg(SPRITE_PALETTE_1_DATA) >> 6 & 0x03) : (getReg(SPRITE_PALETTE_0_DATA) >> 6 & 0x03)],
        };

        unsigned tileData;
        if (cpu->gbMode == DMG) tileData = spriteTile * 16 + relY * 2;
        else tileData = (spriteAttr & 0x08 ? VRAM_BANK_SIZE : 0x0000) + spriteTile * 16 + relY * 2;
        unsigned data = mmu->VRAM[tileData++];
        data |= mmu->VRAM[tileData] << 8;
        if (spriteAttr & 0x20) data = hflip(data);

        for (unsigned bit=0; bit<8; bit++) {
            u8 pLo = ((data & (0x0080 >> bit)) ? 1 : 0);
            u8 pHi = ((data & (0x8000 >> bit)) ? 2 : 0);
            u8 paletteIndex = pLo + pHi;
            // transparent
            if (paletteIndex == 0) continue;

            unsigned x = spriteX + bit;
            if (x < 160) {
                unsigned index = ((getReg(LCDC_Y_COORDINATE) * 160) + x) * 4;
                if (isBitSet(getReg(LCD_CONTROL), BG_DISPLAY)) {
                    if (pixelLine[x].type == 3) continue;
                    if (isBitSet(spriteAttr, 0x80)) {
                        if (pixelLine[x].type == 1 && pixelLine[x].palette > 0) continue;
                    }
                }

                pixelLine[x].palette = paletteIndex;
                pixelLine[x].type = 2;

                if (cpu->gbMode == DMG) {
                    u8 color = dmgPalette[paletteIndex];
                    if (useCustomPalette) {
                        displayState[index] = customPalette[color][0];
                        displayState[index + 1] = customPalette[color][1];
                        displayState[index + 2] = customPalette[color][2];
                    } else {
                        displayState[index] = color;
                        displayState[index + 1] = color;
                        displayState[index + 2] = color;
                    }
                } else {
                    u16 color = getColor(1, spriteAttr, paletteIndex);
                    u8 r, g, b;
                    colorCorrect(color, r, g, b);
                    displayState[index    ] = r;
                    displayState[index + 1] = g;
                    displayState[index + 2] = b;
                }
            }
        }
    }
}

void GPU::colorCorrect(u16 original, u8 &r, u8 &g, u8 &b) {
    u16 rOld =  original & 0x001F;
    u16 gOld = (original & 0x03E0) >> 5;
    u16 bOld = (original & 0x7C00) >> 10;
    u16 rNew = rOld * 26 + gOld *  4 + bOld * 2;
    u16 gNew =             gOld * 24 + bOld * 8;
    u16 bNew = rOld *  6 + gOld *  4 + bOld * 22;
    r = static_cast<u8>((rNew / 1024.f) * 255);
    g = static_cast<u8>((gNew / 1024.f) * 255);
    b = static_cast<u8>((bNew / 1024.f) * 255);
}

u16 GPU::getColor(u8 type, u16 attribute, u16 paletteIndex) {
    assert(cpu->gbMode == CGB);
    // 2 Types x 8 Palettes x 4 Colors x 2 Bytes
    u8 paletteNumber = attribute & 0x07;
    u8 paletteAddress = ((paletteNumber * 4) + paletteIndex) * 2;
    u8 offset = type ? 0x40 : 0x00;
    paletteAddress += offset;
    assert(paletteAddress + 1 <= 0x7F);
    return mmu->PaletteMemory[paletteAddress] | (mmu->PaletteMemory[paletteAddress + 1] << 8);
}

unsigned GPU::hflip(unsigned data) {
    // Hacker's Delight [p. 129]
    unsigned x = (data & 0x5555) << 1 | ((data >> 1) & 0x5555);
    x = (x & 0x3333) << 2 | ((x >> 2) & 0x3333);
    x = (x & 0x0F0F) << 4 | ((x >> 4) & 0x0F0F);
    return x;
}

u8 GPU::getReg(u16 regAddress) {
    return mmu->readByte(regAddress);
}

void GPU::setReg(u16 regAddress, u8 value) {
    // TODO: find a better way to do this
    if (regAddress == LCDC_Y_COORDINATE) mmu->IO[0x44] = value;
    else mmu->writeByte(regAddress, value);
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
    pixelLine.resize(256);
    int counter = 0;
    for (int i=0; i<256; i++) {
        renderBGScanline(true, i);
        u8 r, g, b;
        for (int x=0; x<256; x++) {
            if (cpu->gbMode == CGB) {
                colorCorrect(pixelLine[x].color, r, g, b);
            } else {
                r = pixelLine[x].r, g = pixelLine[x].g, b = pixelLine[x].b;
            }
            backgroundState[counter++] = r;
            backgroundState[counter++] = g;
            backgroundState[counter++] = b;
            counter++;
        }
    }
    pixelLine.resize(160);
    if (!showViewportBorder) return backgroundState.data();

    // render viewport borders
    u8 scrollY = getReg(SCROLL_Y), scrollX = getReg(SCROLL_X);
    int index = 0;
    for (int y=0; y<144; y++) {
        if (y == 0 || y == 143) {
            for (int x=0; x<160; x++) {
                index = (((scrollY + y) % 256) * 256 + ((scrollX + x) % 256)) * 4;
                backgroundState[index] = 0xFF;
                backgroundState[index + 1] = 0x00;
                backgroundState[index + 2] = 0x00;
            }
        } else {
            for (int i=0; i<2; i++) {
                int relX = i ? scrollX : (scrollX + 159) % 256;
                index = (((scrollY + y) % 256) * 256 + relX) * 4;
                backgroundState[index] = 0xFF;
                backgroundState[index + 1] = 0x00;
                backgroundState[index + 2] = 0x00;
            }
        }
    }
    return backgroundState.data();
}

u8* GPU::getTileData(int offset) {
    // build one tile
    u8 tile[64] = {};
    int counter = 0;
    for (int i=0; i<16; i+=2) {
        u8 lowByte = mmu->VRAM[i+offset];
        u8 highByte = mmu->VRAM[i+1+offset];
        for (int j = 7; j >= 0; j--) {
            tile[counter++] = ((lowByte & (0x01 << j)) >> j) | (((highByte & (0x01 << j)) >> j) << 1);
        }
    }
    counter = 0;
    for (u8 t : tile) {
        assert(t < 4);
        tileData[counter++] = colors[t];
        tileData[counter++] = colors[t];
        tileData[counter++] = colors[t];
        tileData[counter++] = 255;
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

void GPU::serialize(phos::serializer &s) {
    s.integer(hitVBlank);
    s.integer(modeclock);
    s.integer(DMATicks);
    s.enumeration(mode);
    s.integer(wyc);
}
