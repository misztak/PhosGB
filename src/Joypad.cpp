#include "Joypad.h"
#include "CPU.h"

Joypad::Joypad(CPU* c) : cpu(c), joypadState(0xFF), lane(BUTTON_KEYS) {}

void Joypad::handleInputDown(u8 key) {
    bool prevStateON = false;
    u8 keyMask = (u8) 1 << key;

    if (!isBitSet(joypadState, keyMask)) {
        prevStateON = true;
    }

    joypadState = clearBit(joypadState, keyMask);

    bool isDirectional = key <= 3;
    if (!prevStateON) {
        if (isDirectional && (lane == DIRECTION_KEYS || lane == BOTH)) cpu->requestInterrupt(INTERRUPT_JOYPAD);
        else if (!isDirectional && (lane == BUTTON_KEYS || lane == BOTH)) cpu->requestInterrupt(INTERRUPT_JOYPAD);
    }

}

void Joypad::handleInputUp(u8 key) {
    joypadState = setBit(joypadState, 1 << key);
}

u8 Joypad::readByte() {
    u8 state = 0xCF;
    if (lane == BUTTON_KEYS) {
        state = 0x10 | (joypadState >> 4);
    } else if (lane == DIRECTION_KEYS){
        state = 0x20 | (joypadState & 0xF);
    }
    return state;
}

void Joypad::writeByte(u8 val) {
    u8 laneVal = val & 0x30;
    if (laneVal == 0x20) lane = DIRECTION_KEYS;
    else if (laneVal == 0x10) lane = BUTTON_KEYS;
    else if (laneVal == 0x30) lane = BOTH;
}
