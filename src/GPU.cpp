#include "GPU.h"

GPU::GPU(CPU* c, MMU* m):
    cpu(c),
    mmu(m),
    mode(HBLANK),
    modeclock(0),
    line(0),
    scrollX(0),
    scrollY(0),
    displayState(DISPLAY_TEXTURE_SIZE, WHITE),
    background(256, std::vector<u8>(256, WHITE)),
    VRAM(8192),
    OAM(160) {}

void GPU::reset() {
    setReg(LCDC_Y_COORDINATE, 0x91);
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
    switch (mode) {
        case READ_OAM:
            if (modeclock >= 80) {
                // enter scanline mode 3 (READ_BOTH)
                modeclock = 0;
                mode = READ_BOTH;
            }
            break;
        case READ_BOTH:
            if (modeclock >= 172) {
                // beginning of HBLANK
                modeclock = 0;
                mode = HBLANK;

                // TODO: write a scanline from the framebuffer + interrupt
            }
            break;
        case HBLANK:
            if (modeclock >= 204) {
                modeclock = 0;
                line++;
                if (line == 143) {
                    // beginning of VBLANK
                    mode = VBLANK;
                    hitVBlank = true;
                    // TODO: more interrupts
                } else {
                    mode = READ_OAM;
                }
            }
            break;
        case VBLANK:
            if (modeclock >= 456) {
                modeclock = 0;
                line++;
                if (line > 153) {
                    mode = READ_OAM;
                    line = 0;
                    // TODO: even more interrupts
                }
            }
            break;
    }
    // TODO: coincidence flag?
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
    return 0;
}

void GPU::writeWord(u16 address, u16 value) {

}

u8 GPU::getReg(u16 regAddress) {
    return mmu->readByte(regAddress);
}

void GPU::setReg(u16 regAddress, u8 value) {
    mmu->writeByte(regAddress, value);
}

u8* GPU::getDisplayState() {
    // TODO: wrap around at edges
    int index = 0;
    for (size_t y = scrollY; y < scrollY + HEIGHT; ++y) {
        for (size_t x = scrollX; x < scrollX + WIDTH; ++x) {
            displayState[index] =       background[y][x];
            displayState[index + 1] =   background[y][x];
            displayState[index + 2] =   background[y][x];
            index += 4;
        }
    }

    return displayState.data();
}
