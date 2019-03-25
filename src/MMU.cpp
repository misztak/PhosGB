#include "MMU.h"

// TODO: add BIOS
MMU::MMU(): inBIOS(false), fatalError(false) {
    // init memory map
    memoryMap[0x0] = &MMU::readROM0; memoryMap[0x1] = &MMU::readROM0;
    memoryMap[0x2] = &MMU::readROM0; memoryMap[0x3] = &MMU::readROM0;
    memoryMap[0x4] = &MMU::readROM1; memoryMap[0x5] = &MMU::readROM1;
    memoryMap[0x6] = &MMU::readROM1; memoryMap[0x7] = &MMU::readROM1;

    memoryMap[0xA] = &MMU::readERAM; memoryMap[0xB] = &MMU::readERAM;
    memoryMap[0xC] = &MMU::readWRAM; memoryMap[0xD] = &MMU::readWRAM;
    memoryMap[0xE] = &MMU::readWRAMshadow;
    memoryMap[0xF] = &MMU::readZRAM;

}

u8 MMU::readByte(u16 address) {
    int location = address & 0xF000;
    if (location == 0xE) {
        printf("Reading shadow wram\n");
    }
    Memory mappedMemory = memoryMap[location];
    return (this->*mappedMemory)(address);
}

u16 MMU::readWord(u16 address) {
    return readByte(address) | (readByte(address + 1) << 8);
}

void MMU::writeByte(u16 address, u8 value) {
    if (address < 0xC000) {
        printf("Attempted to overwrite ROM\n");
        fatalError = true;
        return;
    }
    // TODO: cartridge external RAM
    // TODO: check for off by one errors
    if (address >= 0xC000 && address <= 0xDFFF) {
        workingRAM[address - 0xC000] = value;
    } else if (address >= 0xE000 && address <= 0xFDFF) {
        workingRAM[address - 0xE000] = value;
    } else if (address >= 0xFF00 && address <= 0xFF7F) {
        mappedIO[address - 0xFF00] = value;
    } else if (address >= 0xFF80 && address <= 0xFFFF) {
        zeroPageRAM[address - 0xFF80] = value;
    } else {
        printf("Attempted to write to illegal address %d\n", address);
        fatalError = true;
    }
}

void MMU::writeWord(u16 address, u16 value) {

}

bool MMU::loadROM(std::string& filename, bool isBIOS) {
    std::ifstream file(filename);
    file.seekg(0, std::ifstream::end);
    long length = file.tellg();
    file.seekg(0, std::ifstream::beg);

    if (length == -1) {
        printf("Failed to open file %s\n", filename.c_str());
        return false;
    }
    if (length > INTERNAL_ROM_SIZE) {
        printf("Unsupported ROM size %li\n", length);
        return false;
    }

    char buffer[INTERNAL_ROM_SIZE];
    file.read(&buffer[0], length);

    if (isBIOS) {
        if (length != BIOS_SIZE) {
            printf("Invalid BootROM size: %li\n", length);
            return false;
        }
        std::memcpy(bios, buffer, BIOS_SIZE);
        return true;
    } else {
        std::memcpy(rom0, buffer, ROM0_SIZE);
        std::memcpy(rom1, buffer + ROM0_SIZE, ROM1_SIZE);
        printf("Read file %s, size=%li\n", filename.substr(filename.find_last_of('/')+1, filename.length()).c_str(), length);
        return true;
    }
}

u8 MMU::readROM0(const u16 address) {
    if (inBIOS && address < 0x0100) {
        return bios[address];
    } else if (inBIOS && address == 0x0100) {
        inBIOS = false;
        // TODO: find out if this is correct
    }
    return rom0[address];
}

u8 MMU::readROM1(const u16 address) {
    return rom1[address];
}

u8 MMU::readWRAM(const u16 address) {
    return workingRAM[address & 0x1FFF];
}

u8 MMU::readWRAMshadow(const u16 address) {
    // TODO: what's the point of this?
    return workingRAM[address & 0x1FFF];
}

u8 MMU::readERAM(const u16 address) {
    return externalRAM[address & 0x1FFF];
}

// TODO: rename / refactor this
u8 MMU::readZRAM(const u16 address) {
    int location = address & 0x0F00;
    if (location < 0x0E00) {
        // WRAM shadow
        return workingRAM[address & 0x1FFF];
    } else  if (location == 0x0F00){
        if (address >= 0xFF80) {
            // Zero-page
            return zeroPageRAM[address & 0x7F];
        } else {
            // IO Control
            return mappedIO[address - 0xFF00];
        }
    } else if (location == 0x0E00) {
        printf("Tried to access OAM memory from MMU\n");
        // fall through to generic error message
    }

    printf("Invalid ZRAM address 0x%02X\n", address);
    fatalError = true;
    return 0;
}

