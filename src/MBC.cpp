#include "MBC.h"
#include "MMU.h"

MBC::MBC(MMU *mmu) : mmu(mmu), ROMBankPtr(0), RAMBankPtr(0) {}

NO_MBC::NO_MBC(MMU *mmu) : MBC(mmu) {
    // both bank pointers are always zero
}

u8 NO_MBC::readROMByte(u16 address) {
    return mmu->ROM[address + ROMBankPtr * ROM_BANK_SIZE];
}

void NO_MBC::writeROMByte(u16 address, u8 value) {
    printf("Write to ROM address range\n");
    // writes to ROM are ignored
}

u8 NO_MBC::readRAMByte(u16 address) {
    return mmu->RAM[address + RAMBankPtr * RAM_BANK_SIZE];
}

void NO_MBC::writeRAMByte(u16 address, u8 value) {
    mmu->RAM[address + RAMBankPtr * RAM_BANK_SIZE] = value;
}

