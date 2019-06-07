#ifndef PHOS_MBC_H
#define PHOS_MBC_H

#include <ctime>

#include "Common.h"

class MMU;

class MBC {
public:
    MBC(MMU* mmu);
    virtual ~MBC() = default;
    virtual u8 readROMByte(u16 address) = 0;
    virtual void writeROMByte(u16 address, u8 value) = 0;
    virtual u8 readRAMByte(u16 address) = 0;
    virtual void writeRAMByte(u16 address, u8 value) = 0;
    virtual void saveState(std::ofstream& outfile) = 0;
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
    void saveState(std::ofstream& outfile) override;
};

class MBC1 : public MBC {
public:
    MBC1(MMU* mmu);
    u8 readROMByte(u16 address) override;
    void writeROMByte(u16 address, u8 value) override;
    u8 readRAMByte(u16 address) override;
    void writeRAMByte(u16 address, u8 value) override;
    void saveState(std::ofstream& outfile) override;
private:
    bool RAMEnable;
    u8 ROM_RAM_ModeSelect;
};

class MBC2 : public MBC {
public:
    MBC2(MMU* mmu);
    u8 readROMByte(u16 address) override;
    void writeROMByte(u16 address, u8 value) override;
    u8 readRAMByte(u16 address) override;
    void writeRAMByte(u16 address, u8 value) override;
    void saveState(std::ofstream& outfile) override;
private:
    bool RAMEnable;
};

class MBC3 : public MBC {
public:
    MBC3(MMU* mmu);
    u8 readROMByte(u16 address) override;
    void writeROMByte(u16 address, u8 value) override;
    u8 readRAMByte(u16 address) override;
    void writeRAMByte(u16 address, u8 value) override;
    void saveState(std::ofstream& outfile) override;
    void latchClockData();
public:
    long latchedTime;
private:
    bool RAM_RTC_Enable;
    bool latchInit;
    u8 RAM_RTC_ModeSelect;
    u8 RTCRegisterPtr;
    u8 RTCRegisters[5] = {0};
};

#endif //PHOS_MBC_H
