#ifndef PHOS_JOYPAD_H
#define PHOS_JOYPAD_H

#include "Common.h"

constexpr u16 JOYPAD_ADDRESS = 0xFF00;

class Joypad {
public:
    Joypad();
    void handleInputDown(u8 column, u8 row);
    void handleInputUp(u8 column, u8 row);
    u8 readByte();
    void writeByte(u8 val);
private:
    u8 rows[2] = { 0x0F };
    u8 column;
};

#endif //PHOS_JOYPAD_H
