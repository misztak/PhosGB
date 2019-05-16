#include "MMU.h"
#include "CPU.h"
#include "GPU.h"

MMU::MMU() :
    cpu(nullptr),
    gpu(nullptr),
    inBIOS(true),
    BIOS(BIOS_SIZE, 0),
    ROM_0(ROM_BANK_SIZE, 0),
    // init ROM and RAM with capacity of NO_MBC cartridge
    ROM(ROM_BANK_SIZE, 0),
    RAM(RAM_BANK_SIZE, 0),
    WRAM(WRAM_SIZE, 0),
    IO(IO_SIZE, 0),
    ZRAM(ZRAM_SIZE, 0),
    VRAM(VRAM_SIZE, 0),
    OAM(OAM_SIZE, 0),
    mbc(nullptr) {
    initTables();
}

bool MMU::init(std::string& romPath, std::string& biosPath) {
    std::vector<u8> buffer;

    if (!biosPath.empty()) {
        if (!loadFile(biosPath, FileType::BIOS, buffer)) return false;
        std::copy_n(buffer.begin(), BIOS_SIZE, BIOS.begin());
    } else {
        inBIOS = false;
    }

    buffer.clear();
    if (!loadFile(romPath, FileType::ROM, buffer)) return false;

    u8 cartridgeType = buffer[0x147];
    u8 RAMType = buffer[0x149];
    printf("Read file %s, size=%li\n", romPath.substr(romPath.find_last_of('/')+1, romPath.length()).c_str(), buffer.size());

    if (buffer[0x143] == 0xC0) {
        printf("Gameboy Color cartridges not supported\n");
        return false;
    }
    if (buffer[0x143] == 0x80) {
        printf("Gameboy Color cartridge with Non-CGB-Mode\n");
    }

    switch (cartridgeType) {
        case 0x00:
        case 0x08:
        case 0x09:
            mbc = std::make_unique<NO_MBC>(this);
            break;
        case 0x01:
        case 0x02:
        case 0x03:
        case 0xFF:
            mbc = std::make_unique<MBC1>(this);
            break;
        case 0x05:
        case 0x06:
            mbc = std::make_unique<MBC2>(this);
            break;
        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
        case 0xFC:
            printCartridgeInfo(buffer);
            printf("No support for MBC3 cartridges yet\n");
            return false;
        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
            printCartridgeInfo(buffer);
            printf("No support for MBC5 cartridges yet\n");
            return false;
        case 0x0B:
        case 0x0C:
        case 0x0D:
        case 0x20:
        case 0x22:
        case 0xFD:
        case 0xFE:
            printCartridgeInfo(buffer);
            printf("No support for cartridge type '%s'\n", cartridgeTypes[cartridgeType].c_str());
            return false;
        default:
            printf("Unknown cartridge type 0x%2X detected\n", cartridgeType);
            return false;
    }

    if (!mbc) {
        printf("Failed to initialize MBC for cartridge type 0x%2X\n", cartridgeType);
        return false;
    }

    if (RAMSizeTypes.count(RAMType) == 0) {
        printf("Invalid RAM type: %d\n", RAMType);
        return false;
    }

    std::copy_n(buffer.begin(), ROM_BANK_SIZE, ROM_0.begin());
    ROM.resize(buffer.size() - ROM_BANK_SIZE);
    std::copy(buffer.begin() + ROM_BANK_SIZE, buffer.end(), ROM.begin());

    if (cartridgeType == 0x05 || cartridgeType == 0x06) {
        RAM.resize(512, 0);
    } else {
        RAM.resize(RAMSizeTypes[RAMType], 0);
    }
    std::fill(RAM.begin(), RAM.end(), 0);

    std::fill(WRAM.begin(), WRAM.end(), 0);
    std::fill(IO.begin(), IO.end(), 0);
    std::fill(ZRAM.begin(), ZRAM.end(), 0);
    std::fill(VRAM.begin(), VRAM.end(), 0);
    std::fill(OAM.begin(), OAM.end(), 0);

    if (cartridgeTypes[cartridgeType].find("RAM+BATTERY")) {
        // look for a .sav file of valid size
        std::string saveName = romPath.substr(romPath.find_last_of('/') + 1, romPath.length());
        saveName.append(".sav");
        std::vector<u8> saveBuffer;
        if (!loadFile(saveName, FileType::SRAM, saveBuffer)) {
            printf("Could not open .sav file %s. Continuing without it.\n", saveName.c_str());
        } else {
            std::copy_n(saveBuffer.begin(), RAM.size(), RAM.begin());
            printf("Successfully loaded SRAM from .sav file %s\n", saveName.c_str());
        }
    }

    printCartridgeInfo(buffer);
    return true;
}

bool MMU::loadFile(std::string& path, FileType fileType, std::vector<u8>& buffer) {
    std::ifstream file(path);
    if (!file || !file.good()) {
        printf("Failed to open file %s\n", path.c_str());
        return false;
    }
    file.seekg(0, std::ifstream::end);
    long length = file.tellg();
    file.seekg(0, std::ifstream::beg);

    if (length == -1 || length == 0x7FFFFFFFFFFFFFFF) {
        printf("Failed to load file %s\n", path.c_str());
        return false;
    }

    buffer.resize(length);
    file.read((char *) buffer.data(), length);

    switch (fileType) {
        case FileType::BIOS:
            if (length != BIOS_SIZE) {
                printf("Invalid BootROM size: %li\n", length);
                return false;
            }
            return true;
        case FileType::ROM:
            // check type id and underlying value
            if (ROMSizeTypes.count(buffer[0x148]) && ROMSizeTypes[buffer[0x148]] == length) return true;
            // TODO: turn this into an option or a warning
            printf("Cartridge type %d with size %li is invalid\n", buffer[0x148], length);
            return false;
        case FileType::SRAM:
            // RAM is already resized at this point so just compare with that
            if (buffer.size() != RAM.size()) {
                printf("Invalid size of .sav file. Should be %li but detected %li\n", RAM.size(), buffer.size());
                return false;
            }
            return true;
    }
    return false;
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
            return mbc->readROMByte(address - 0x4000);
        case 0x8000:
        case 0x9000:
            return VRAM[address - 0x8000];
        case 0xA000:
        case 0xB000:
            return mbc->readRAMByte(address - 0xA000);
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
                return 0x00;
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
        case 0x4000:
        case 0x5000:
        case 0x6000:
        case 0x7000:
            mbc->writeROMByte(address, value);
            return;
        case 0x8000:
        case 0x9000:
            VRAM[address - 0x8000] = value;
            return;
        case 0xA000:
        case 0xB000:
            mbc->writeRAMByte(address - 0xA000, value);
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
                if (address == 0xFF04) {            // reset divider counter on write
                    IO[address - 0xFF00] = 0;
                } else if (address == 0xFF40) {     // LCD Control
                    u8 wasLCDEnabled = IO[0x40] & LCD_DISPLAY_ENABLE;
                    IO[0x40] = value;
                    if (wasLCDEnabled && !(value & LCD_DISPLAY_ENABLE)) {
                        assert(gpu->getMode() == VBLANK && "Tried to disable display outside of VBLANK period\n");
                        gpu->setBGColor(0xFF);      // set display to all white
                        // reset some STAT values
                        // TODO: see if this is right
                        // i.e. compare with "https://www.reddit.com/r/EmuDev/comments/6r6gf3/gb_pokemon_gold_spews_unexpected_values_at_mbc/dl5c0ub"
                        IO[LCDC_Y_COORDINATE - 0xFF00] = 153;
                        gpu->modeclock = 456;
                        gpu->setMode(VBLANK);
                    }
                } else if (address == 0xFF41) {     // LCDC STAT
                    IO[address - 0xFF00] = (value & (u8) 0xF8) | (IO[address - 0xFF00] & (u8) 0x07);
                } else if (address == 0xFF44) {     // LY (line) (Read-only)
                    return;
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

void MMU::initTables() {
    // NO MBC
    cartridgeTypes[0x00] = "ROM ONLY";
    cartridgeTypes[0x08] = "ROM+RAM";
    cartridgeTypes[0x09] = "ROM+RAM+BATTERY";
    // MBC1
    cartridgeTypes[0x01] = "MBC1";
    cartridgeTypes[0x02] = "MBC1+RAM";
    cartridgeTypes[0x03] = "MBC1+RAM+BATTERY";
    cartridgeTypes[0xFF] = "HuC1+RAM+BATTERY";
    // MBC2
    cartridgeTypes[0x05] = "MBC2";
    cartridgeTypes[0x06] = "MBC2+BATTERY";
    // MBC3
    cartridgeTypes[0x11] = "MBC3";
    cartridgeTypes[0x12] = "MBC3+RAM";
    cartridgeTypes[0x13] = "MBC3+RAM+BATTERY";
    cartridgeTypes[0x0F] = "MBC3+TIMER+BATTERY";
    cartridgeTypes[0x10] = "MBC3+TIMER+RAM+BATTERY";
    cartridgeTypes[0xFC] = "POCKET CAMERA";
    // MBC5
    cartridgeTypes[0x19] = "MBC5";
    cartridgeTypes[0x1A] = "MBC5+RAM";
    cartridgeTypes[0x1B] = "MBC5+RAM+BATTERY";
    cartridgeTypes[0x1C] = "MBC5+RUMBLE";
    cartridgeTypes[0x1D] = "MBC5+RUMBLE+RAM";
    cartridgeTypes[0x1E] = "MBC5+RUMBLE+RAM+BATTERY";
    // MISC
    cartridgeTypes[0x0B] = "MMM01";
    cartridgeTypes[0x0C] = "MMM01+RAM";
    cartridgeTypes[0x0D] = "MMM01+RAM+BATTERY";
    cartridgeTypes[0x20] = "MBC6+RAM+BATTERY+FLASH";
    cartridgeTypes[0x22] = "MBC7+RAM+BATTERY";
    cartridgeTypes[0x55] = "GG";
    cartridgeTypes[0x56] = "GS3";
    cartridgeTypes[0xFD] = "BANDAI TAMA5";
    cartridgeTypes[0xFE] = "HuC3";

    ROMSizeTypes[0x00] = 32768;
    ROMSizeTypes[0x01] = 65536;
    ROMSizeTypes[0x02] = 131072;
    ROMSizeTypes[0x03] = 262144;
    ROMSizeTypes[0x04] = 524288;
    ROMSizeTypes[0x05] = 1048576;
    ROMSizeTypes[0x06] = 2097152;
    ROMSizeTypes[0x07] = 4194304;
    ROMSizeTypes[0x08] = 8388608;
    // ROM size types 0x52, 0x53 and 0x54 don't exist in official games but are mentioned in some documents
    // ROMSizeTypes[0x52] = 1179648;
    // ROMSizeTypes[0x53] = 1310720;
    // ROMSizeTypes[0x54] = 1572864;

    RAMSizeTypes[0x00] = 0;
    RAMSizeTypes[0x01] = 2048;
    RAMSizeTypes[0x02] = 8192;
    RAMSizeTypes[0x03] = 32768;
    RAMSizeTypes[0x05] = 65536;
}

void MMU::printCartridgeInfo(std::vector<u8>& buffer) {
    cartridgeTitle = std::string(&buffer[0x134], &buffer[0x134] + 0xF);
    u8 cartridgeType = buffer[0x147];

    printf("\n");
    printf("Game Title:          %s\n", cartridgeTitle.c_str());
    printf("Cartridge Type:      0x%02X (%s)\n", cartridgeType, cartridgeTypes[cartridgeType].c_str());
    printf("ROM Size Type:       0x%02X (%d Byte)\n", buffer[0x148], ROMSizeTypes[buffer[0x148]]);
    printf("RAM Size Type:       0x%02X (%d Byte)", buffer[0x149], RAMSizeTypes[buffer[0x149]]);
    if (cartridgeType == 0x05 || cartridgeType == 0x06) printf(" -- (%li Byte MBC2 internal)\n", RAM.size());
    else printf("\n");
    printf("Destination Code:    %d", buffer[0x14A]);
    buffer[0x14A] ? printf(" (Non-Japanese)\n") : printf(" (Japanese)\n");
    u8 licenceCode = buffer[0x14B];
    if (licenceCode == 0x33) {
        std::string newCode = std::string(&buffer[0x144], &buffer[0x144] + 1);
        printf("Licensee Code (New): %s\n", newCode.c_str());
    } else {
        printf("Licensee Code (Old): 0x%02X\n", licenceCode);
    }
    printf("Game Version:        %d\n", buffer[0x14C]);

    u8 headerCRC = buffer[0x14D];
    printf("Header Checksum:     0x%02X", headerCRC);
    u8 x = 0;
    for (int i=0x134; i<=0x14C; i++) x = x - buffer[i] -1;
    if (x != headerCRC) {
        printf("   [INVALID - actual checksum is 0x%02X, a real Gameboy would halt execution]\n", x);
    } else {
        printf("   [VALID]\n");
    }

    u16 globalCRC = (buffer[0x14E] << 8) | buffer[0x14F];
    printf("Global Checksum:     0x%04X", globalCRC);
    u16 g = 0;
    for (u8 i : buffer) g += i;
    g -= buffer[0x14E];
    g -= buffer[0x14F];
    if (g != globalCRC) {
        printf(" [INVALID - actual checksum is 0x%04X, but a real Gameboy would not care]\n", g);
    } else {
        printf(" [VALID]\n");
    }
    printf("\n");
}
