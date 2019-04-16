#include "Joypad.h"

Joypad::Joypad() : column(0) {}

void Joypad::handleInputDown(u8 column, u8 row) {
    rows[column] &= row;
}

void Joypad::handleInputUp(u8 column, u8 row) {
    rows[column] |= row;
}

u8 Joypad::readByte() {
    switch (column) {
        case 0x10:
            return rows[0];
        case 0x20:
            return rows[1];
        default:
            printf("Invalid joypad column %d\n", column);
            return 0;
    }
}

void Joypad::writeByte(u8 val) {
    column = val & (u8) 0x30;
}
