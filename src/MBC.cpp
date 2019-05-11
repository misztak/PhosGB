#include "MBC.h"
#include "MMU.h"

MBC::MBC(MMU *mmu) : mmu(mmu), ROMBankPtr(0), RAMBankPtr(0) {}

// NO_MBC

NO_MBC::NO_MBC(MMU *mmu) : MBC(mmu) {
    // both bank pointers are always zero
}

u8 NO_MBC::readROMByte(u16 address) {
    return mmu->ROM[address];
}

void NO_MBC::writeROMByte(u16 address, u8 value) {
    printf("Write to ROM address range\n");
    // writes to ROM are ignored
}

u8 NO_MBC::readRAMByte(u16 address) {
    return mmu->RAM[address];
}

void NO_MBC::writeRAMByte(u16 address, u8 value) {
    mmu->RAM[address] = value;
}

// MBC1

MBC1::MBC1(MMU *mmu) : MBC(mmu), RAMEnable(false), ROM_RAM_ModeSelect(0) {}

u8 MBC1::readROMByte(u16 address) {
    return mmu->ROM[address + ROMBankPtr * ROM_BANK_SIZE];
}

void MBC1::writeROMByte(u16 address, u8 value) {
    u16 type = address & 0xF000;
    u8 v;
    switch (type) {
        case 0x0000:
        case 0x1000:
            RAMEnable = (value & 0x0F) == 0x0A;
            break;
        case 0x2000:
        case 0x3000:
            assert(value <= 0x1F);
            v = value;
            if (v == 0x00) v = 0x01;
            ROMBankPtr = (ROMBankPtr & 0x60) | (v & 0x1F);
            ROMBankPtr--;
            break;
        case 0x4000:
        case 0x5000:
            if (ROM_RAM_ModeSelect) RAMBankPtr = value & 0x03;
            else ROMBankPtr = (ROMBankPtr & 0x1F) | ((value & 0x03) << 5);
            break;
        case 0x6000:
        case 0x7000:
            if (value) {
                // RAM Banking Mode
                ROM_RAM_ModeSelect = 1;
                ROMBankPtr &= 0x1F;
            } else {
                // ROM Banking Mode
                ROM_RAM_ModeSelect = 0;
                RAMBankPtr = 0;
            }
            break;
        default:
            printf("Invalid MBC1 Control Register address: 0x%4X\n", address);
    }
}

u8 MBC1::readRAMByte(u16 address) {
    if (RAMEnable) return mmu->RAM[address + RAMBankPtr * RAM_BANK_SIZE];
    return 0;
}

void MBC1::writeRAMByte(u16 address, u8 value) {
    if (RAMEnable) mmu->RAM[address + RAMBankPtr * RAM_BANK_SIZE] = value;
}
