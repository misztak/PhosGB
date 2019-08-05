#ifndef PHOS_JOYPAD_H
#define PHOS_JOYPAD_H

#include "Common.h"

class CPU;

constexpr u16 JOYPAD_ADDRESS = 0xFF00;

enum JOYPAD_LANE { BUTTON_KEYS, DIRECTION_KEYS, BOTH };

class Joypad {
public:
    Joypad(CPU* c);
    void reset();
    void handleInputDown(u8 key);
    void handleInputUp(u8 key);
    u8 readByte();
    void writeByte(u8 val);

    void serialize(phos::serializer& s);
private:
    CPU* cpu;
    u8 joypadState;
    JOYPAD_LANE lane;
};

#endif //PHOS_JOYPAD_H
