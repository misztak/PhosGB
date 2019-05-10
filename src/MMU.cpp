#include "MMU.h"

MMU::MMU() :
    inBIOS(true),
    BIOS(BIOS_SIZE, 0),
    ROM_0(ROM_BANK_SIZE, 0),
    // init ROM and RAM with capacity of MBC0 cartridge
    ROM(ROM_BANK_SIZE, 0),
    RAM(RAM_BANK_SIZE, 0),
    WRAM(WRAM_SIZE, 0),
    IO(IO_SIZE, 0),
    ZRAM(ZRAM_SIZE, 0),
    VRAM(VRAM_SIZE, 0),
    OAM(OAM_SIZE, 0) {}

bool MMU::init(std::string& romPath, std::string& biosPath) {
    std::vector<u8> buffer;

    if (!biosPath.empty()) {
        if (!loadFile(biosPath, true, buffer)) return false;
        std::copy_n(buffer.begin(), BIOS_SIZE, BIOS.begin());
    } else {
        inBIOS = false;
    }

    buffer.clear();
    if (!loadFile(romPath, false, buffer)) return false;

    std::copy_n(buffer.begin(), ROM_BANK_SIZE, ROM_0.begin());
    std::copy(buffer.begin() + ROM_BANK_SIZE, buffer.end(), ROM.begin());
    printf("Read file %s, size=%li\n", romPath.substr(romPath.find_last_of('/')+1, romPath.length()).c_str(), buffer.size());

    return true;
}

bool MMU::loadFile(std::string& path, bool isBIOS, std::vector<u8>& buffer) {
    std::ifstream file(path);
    file.seekg(0, std::ifstream::end);
    long length = file.tellg();
    file.seekg(0, std::ifstream::beg);

    if (length == -1) {
        printf("Failed to open file %s\n", path.c_str());
        return false;
    }

    // TODO: max length limit
    buffer.resize(length);
    file.read((char *) buffer.data(), length);

    if (isBIOS) {
        if (length != BIOS_SIZE) {
            printf("Invalid BootROM size: %li\n", length);
            return false;
        }
        return true;
    } else {
        // TODO: check if length fits cartridge type
        return true;
    }
}

u8 MMU::readByte(u16 address) {
    switch (address & 0xF000) {
        case 0x0000:
        case 0x1000:
        case 0x2000:
        case 0x3000:
            if (inBIOS && address < 0x0100) return BIOS[address];
            else if (inBIOS && address == 0x0100) inBIOS = false;
            return ROM_0[address];
        case 0x4000:
        case 0x5000:
        case 0x6000:
        case 0x7000:
            return ROM[address - 0x4000];
        case 0x8000:
        case 0x9000:
            return VRAM[address - 0x8000];
        case 0xA000:
        case 0xB000:
            return RAM[address - 0xA000];
        case 0xC000:
        case 0xD000:
            return WRAM[address - 0xC000];
        case 0xE000:
            return WRAM[address - 0xE000];
        case 0xF000:
            if (address <= 0xFDFF) return WRAM[address - 0xE000];
            if (address <= 0xFE9F) {
                return OAM[address - 0xFE00];
            }
            if (address <= 0xFEFF) {
                // unused memory
                return 0xFF;
            }
            if (address <= 0xFF7F) {
                return IO[address - 0xFF00];
            }
            return ZRAM[address - 0xFF80];
        default:
            printf("You should not be here\n");
            return 0xFF;
    }
}

u16 MMU::readWord(u16 address) {
    assert(address + 1 <= 0xFFFF);
    return readByte(address) | (readByte(address + 1) << 8);
}

void MMU::writeByte(u16 address, u8 value) {
    switch (address & 0xF000) {
        case 0x0000:
        case 0x1000:
        case 0x2000:
        case 0x3000:
            printf("Write to ROM_0 address range\n");
            return;
        case 0x4000:
        case 0x5000:
        case 0x6000:
        case 0x7000:
            printf("Write to ROM_x address range\n");
            return;
        case 0x8000:
        case 0x9000:
            VRAM[address - 0x8000] = value;
            return;
        case 0xA000:
        case 0xB000:
            RAM[address - 0xA000] = value;
            return;
        case 0xC000:
        case 0xD000:
            WRAM[address - 0xC000] = value;
            return;
        case 0xF000:
            if (address <= 0xFDFF) return;
            if (address <= 0xFE9F) {
                OAM[address - 0xFE00] = value;
                return;
            }
            if (address <= 0xFEFF) {
                // unused memory
                return;
            }
            if (address <= 0xFF7F) {
                assert(address != 0xFF00);
                if (address == 0xFF04) {
                    // reset divider counter on write
                    IO[address - 0xFF00] = 0;
                } else if (address == 0xFF41) {
                    IO[address - 0xFF00] = (value & (u8) 0xF8) | (IO[address - 0xFF00] & (u8) 0x07);
                } else if (address == 0xFF46) {
                    // start DMA transfer
                    u16 source = value << (u16) 8;
                    for (int i=0; i<0xA0; i++) {
                        writeByte(0xFE00 + i, readByte(source + i));
                    }
                    IO[address - 0xFF00] = value;
                } else {
                    IO[address - 0xFF00] = value;
                }
                return;
            }
            ZRAM[address - 0xFF80] = value;
            return;
        default:
            return;
    }
}

void MMU::writeWord(u16 address, u16 value) {
    assert(address + 1 <= 0xFFFF);
    u8 low = value & 0xFF;
    u8 high = (value >> 8) & 0xFF;
    writeByte(address, low);
    writeByte(address + 1, high);
}

u8 MMU::readBankedROM(u16 address) {
    return 0;
}

u8 MMU::readBankedRAM(u16 address) {
    return 0;
}

void MMU::writeBankedRAM(u16 address, u8 value) {

}
