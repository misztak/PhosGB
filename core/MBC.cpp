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
    return 0xFF;
}

void MBC1::writeRAMByte(u16 address, u8 value) {
    if (RAMEnable) mmu->RAM[address + RAMBankPtr * RAM_BANK_SIZE] = value;
}

// MBC2

MBC2::MBC2(MMU *mmu) : MBC(mmu), RAMEnable(false) {}

u8 MBC2::readROMByte(u16 address) {
    return mmu->ROM[address + ROMBankPtr * ROM_BANK_SIZE];
}

void MBC2::writeROMByte(u16 address, u8 value) {
    u16 type = address & 0xF000;
    switch (type) {
        case 0x0000:
        case 0x1000:
            if ((address & 0x0100) != 0) break;
            RAMEnable = (value & 0xF) == 0x0A;
            break;
        case 0x2000:
        case 0x3000:
            if ((address & 0x0100) == 0) break;
            assert((value & 0x0F) != 0);
            ROMBankPtr = (value & 0x0F) - 1;
            break;
        default:
            printf("Invalid MBC2 Control Register address: 0x%4X\n", address);
    }
}

u8 MBC2::readRAMByte(u16 address) {
    assert(address < 0x0200);
    if (RAMEnable) return mmu->RAM[address] & 0x0F;
    return 0xFF;
}

void MBC2::writeRAMByte(u16 address, u8 value) {
    assert(address < 0x0200);
    if (RAMEnable) mmu->RAM[address] = value & 0x0F;
}

// MBC 3

MBC3::MBC3(MMU *mmu) : MBC(mmu), RAM_RTC_Enable(false), RAM_RTC_ModeSelect(0), RTCRegisterPtr(0) {}

u8 MBC3::readROMByte(u16 address) {
    return mmu->ROM[address + ROMBankPtr * ROM_BANK_SIZE];
}

void MBC3::writeROMByte(u16 address, u8 value) {
    u16 type = address & 0xF000;
    switch (type) {
        case 0x0000:
        case 0x1000:
            RAM_RTC_Enable = (value & 0x0F) == 0x0A;
            break;
        case 0x2000:
        case 0x3000:
            ROMBankPtr = value & 0x7F;
            if (ROMBankPtr != 0) ROMBankPtr--;
            break;
        case 0x4000:
        case 0x5000:
            if (value <= 0x07) {
                RAMBankPtr = value;
                RAM_RTC_ModeSelect = 0;
            } else if (value <= 0x0C){
                RAM_RTC_ModeSelect = 1;
                RTCRegisterPtr = value - 0x08;
                printf("Tried to access RTC register\n");
            } else {
                printf("Invalid MBC3 RAM_RTC_Register_Select value: 0x%2X\n", value);
            }
            break;
        case 0x6000:
        case 0x7000:
            printf("Tried to access Latch Clock Data in MBC3\n");
            break;
        default:
            printf("Invalid MBC3 Control Register address: 0x%4X\n", address);
    }
}

u8 MBC3::readRAMByte(u16 address) {
    if (!RAM_RTC_Enable) return 0xFF;
    if (RAM_RTC_ModeSelect == 0) {
        // read from RAM bank
        return mmu->RAM[address + RAMBankPtr * RAM_BANK_SIZE];
    } else {
        // read from RTC register
        return RTCRegisters[RTCRegisterPtr];
    }
}

void MBC3::writeRAMByte(u16 address, u8 value) {
    if (!RAM_RTC_Enable) return;
    if (RAM_RTC_ModeSelect == 0) {
        // write to RAM bank
        mmu->RAM[address + RAMBankPtr * RAM_BANK_SIZE] = value;
    } else {
        // write to RTC register
        RTCRegisters[RTCRegisterPtr] = value;
    }
}
