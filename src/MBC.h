#ifndef PHOS_MBC_H
#define PHOS_MBC_H

#include "Common.h"

class MMU;

class MBC {
public:
    MBC(MMU* mmu);
    virtual u8 readROMByte(u16 address) = 0;
    virtual void writeROMByte(u16 address, u8 value) = 0;
    virtual u8 readRAMByte(u16 address) = 0;
    virtual void writeRAMByte(u16 address, u8 value) = 0;
public:
    MMU* mmu;
    u16 ROMBankPtr;
    u16 RAMBankPtr;
};

class NO_MBC : public MBC {
public:
    NO_MBC(MMU* mmu);
    u8 readROMByte(u16 address) override;
    void writeROMByte(u16 address, u8 value) override;
    u8 readRAMByte(u16 address) override;
    void writeRAMByte(u16 address, u8 value) override;
};

class MBC1 : public MBC {
public:
    MBC1(MMU* mmu);
    u8 readROMByte(u16 address) override;
    void writeROMByte(u16 address, u8 value) override;
    u8 readRAMByte(u16 address) override;
    void writeRAMByte(u16 address, u8 value) override;
private:
    bool RAMEnable;
    u8 ROM_RAM_ModeSelect;
};

#endif //PHOS_MBC_H
