#ifndef PHOS_MBC_HPP
#define PHOS_MBC_HPP

#include <ctime>

#include "Common.hpp"

class MMU;

class MBC {
public:
    MBC(MMU* mmu);
    virtual ~MBC() = default;
    virtual u8 readROMByte(u16 address) = 0;
    virtual void writeROMByte(u16 address, u8 value) = 0;
    virtual u8 readRAMByte(u16 address) = 0;
    virtual void writeRAMByte(u16 address, u8 value) = 0;
    virtual void serialize(phos::serializer& s) = 0;
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
    void serialize(phos::serializer& s) override;
};

class MBC1 : public MBC {
public:
    MBC1(MMU* mmu);
    u8 readROMByte(u16 address) override;
    void writeROMByte(u16 address, u8 value) override;
    u8 readRAMByte(u16 address) override;
    void writeRAMByte(u16 address, u8 value) override;
    void serialize(phos::serializer& s) override;
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
    void serialize(phos::serializer& s) override;
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
    void serialize(phos::serializer& s) override;
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

class MBC5 : public MBC {
public:
    MBC5(MMU* mmu, bool hasRumble);
    u8 readROMByte(u16 address) override;
    void writeROMByte(u16 address, u8 value) override;
    u8 readRAMByte(u16 address) override;
    void writeRAMByte(u16 address, u8 value) override;
    void serialize(phos::serializer& s) override;
private:
    bool RAMEnable;
    bool hasRumble;
};

#endif //PHOS_MBC_HPP
