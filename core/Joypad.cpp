#include "Joypad.h"
#include "CPU.h"

Joypad::Joypad(CPU* c) : cpu(c), joypadState(0xFF), lane(BUTTON_KEYS) {}

void Joypad::reset() {
    joypadState = 0xFF;
    lane = BUTTON_KEYS;
}

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
    u8 state = 0xFF;
    if (lane == BUTTON_KEYS) {
        state = 0xD0 | (joypadState >> 4);
    } else if (lane == DIRECTION_KEYS){
        state = 0xE0 | (joypadState & 0xF);
    }
    return state;
}

void Joypad::writeByte(u8 val) {
    u8 laneVal = val & 0x30;
    if (laneVal == 0x20) lane = DIRECTION_KEYS;
    else if (laneVal == 0x10) lane = BUTTON_KEYS;
    else if (laneVal == 0x30) lane = BOTH;
}

void Joypad::saveState(std::ofstream& outfile) {
    outfile.write(WRITE_V(joypadState), 1);
    outfile.write(WRITE_V(lane), sizeof(JOYPAD_LANE));
}

void Joypad::loadState(std::vector<u8>& buffer) {
    // Joypad Offset == 0x32
    joypadState = READ_U8(&buffer[0x32]);
    lane = static_cast<JOYPAD_LANE>(READ_S32(&buffer[0x33]));
}
