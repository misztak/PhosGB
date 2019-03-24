#include "GPU.h"

GPU::GPU(CPU* c, MMU* m):
    mode(HBLANK),
    modeclock(0),
    line(0),
    displayState(DISPLAY_TEXTURE_SIZE) {
    cpu = c;
    mmu = m;

    for (size_t i=1; i<=DISPLAY_TEXTURE_SIZE; i++) {
        if (i % 4 == 0) displayState[i-1] = 255;
        else displayState[i-1] = 127;
    }

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

                // TODO: write a scanline from the framebuffer
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
                }
            }
            break;
    }
}

u8* GPU::getDisplayState() {
    return displayState.data();
}
