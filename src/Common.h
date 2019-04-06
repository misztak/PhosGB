#ifndef PHOS_COMMON_H
#define PHOS_COMMON_H

#include <string>
#include <vector>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

constexpr u32 WIDTH = 160;
constexpr u32 HEIGHT = 144;
constexpr u32 DISPLAY_TEXTURE_SIZE = WIDTH * HEIGHT * 4;

// addresses of interrupt service routines
constexpr u16 INTERRUPT_VBLANK = 0x40;
constexpr u16 INTERRUPT_LCD_STAT = 0x48;
constexpr u16 INTERRUPT_TIMER = 0x50;
constexpr u16 INTERRUPT_SERIAL = 0x58;
constexpr u16 INTERRUPT_JOYPAD = 0x60;

inline bool isBitSet(u8 value, u8 bitMask) {
    return value & bitMask;
}

inline u8 setBit(u8 value, u8 bitMask) {
    return value | bitMask;
}

inline u8 clearBit(u8 value, u8 bitMask) {
    return value & ~bitMask;
}

#endif //PHOS_COMMON_H
