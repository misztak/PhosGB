#include "MMU.hpp"
#include "CPU.hpp"
#include "GPU.hpp"

MMU::MMU() :
    cpu(nullptr),
    gpu(nullptr),
    inBIOS(true),
    runBIOS(false),
    WRAMBankPtr(1),
    VRAMBankPtr(0),
    BIOS(BIOS_SIZE_DMG, 0),
    ROM_0(ROM_BANK_SIZE, 0),
    // init ROM and RAM with capacity of NO_MBC cartridge
    ROM(ROM_BANK_SIZE, 0),
    RAM(RAM_BANK_SIZE, 0),
    WRAM(WRAM_SIZE, 0),
    IO(IO_SIZE, 0),
    ZRAM(ZRAM_SIZE, 0),
    VRAM(VRAM_SIZE, 0),
    OAM(OAM_SIZE, 0),
    PaletteMemory(128, 0xFF),
    mbc(nullptr),
    DMACounter(0),
    GDMACounter(0),
    HDMACounter(0)
    {
    VramDma.reset();
    initTables();
}

bool MMU::init(std::string& romPath, std::string& biosPath) {
    std::vector<u8> buffer;
    if (!loadFile(romPath, FileType::ROM, buffer)) return false;

    u8 cartridgeType = buffer[0x147];
    u8 RAMType = buffer[0x149];
    Log(I, "Read file %s, size=%li\n", romPath.substr(romPath.find_last_of('/')+1, romPath.length()).c_str(), buffer.size());

    cpu->gbMode = DMG;
    if (cpu->runCGBinDMGMode && buffer[0x143] == 0x80) {
        Log(I, "Running CGB ROM in DMG Mode\n");
    } else if (buffer[0x143] == 0x80 || buffer[0x143] == 0xC0) {
        cpu->gbMode = CGB;
        Log(I, "Running ROM in CGB Mode\n");
    }

    std::vector<u8> biosBuffer;
    if (runBIOS && !biosPath.empty() && loadFile(biosPath, FileType::BIOS, biosBuffer)) {
        if (cpu->gbMode == DMG) {
            BIOS.resize(BIOS_SIZE_DMG);
            std::copy_n(biosBuffer.begin(), BIOS_SIZE_DMG, BIOS.begin());
        } else if (cpu->gbMode == CGB) {
            BIOS.resize(BIOS_SIZE_CGB);
            std::copy_n(biosBuffer.begin(), BIOS_SIZE_CGB, BIOS.begin());
        }
        inBIOS = true;
    } else {
        inBIOS = false;
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
            mbc = std::make_unique<MBC3>(this);
            break;
        case 0x19:
        case 0x1A:
        case 0x1B:
            // MBC5 without rumble motor
            mbc = std::make_unique<MBC5>(this, false);
            break;
        case 0x1C:
        case 0x1D:
        case 0x1E:
            // MBC5 with rumble motor
            mbc = std::make_unique<MBC5>(this, true);
            break;
        case 0x0B:
        case 0x0C:
        case 0x0D:
        case 0x20:
        case 0x22:
        case 0xFD:
        case 0xFE:
            printCartridgeInfo(buffer);
            Log(W, "No support for cartridge type '%s'\n", cartridgeTypes[cartridgeType].c_str());
            return false;
        default:
            Log(W, "Unknown cartridge type 0x%2X detected\n", cartridgeType);
            return false;
    }

    if (!mbc) {
        Log(W, "Failed to initialize MBC for cartridge type 0x%2X\n", cartridgeType);
        return false;
    }

    if (RAMSizeTypes.count(RAMType) == 0) {
        Log(W, "Invalid RAM type: %d\n", RAMType);
        return false;
    }

    if (RAMSizeTypes[RAMType] == 0 && cartridgeTypes[cartridgeType].find("RAM") != std::string::npos &&
        cartridgeTypes[cartridgeType].find("MBC2") == std::string::npos) {
        Log(W, "Cartridge type and RAM size type don't match\n");
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

    // resize and clear all internal storage vectors
    if (cpu->gbMode == DMG) {
        WRAM.resize(WRAM_SIZE);
        VRAM.resize(VRAM_SIZE);
    } else if (cpu->gbMode == CGB) {
        WRAM.resize(WRAM_BANK_SIZE * 8);
        VRAM.resize(VRAM_SIZE * 2);
    }
    std::fill(WRAM.begin(), WRAM.end(), 0);
    std::fill(IO.begin(), IO.end(), 0);
    std::fill(ZRAM.begin(), ZRAM.end(), 0);
    std::fill(VRAM.begin(), VRAM.end(), 0);
    std::fill(OAM.begin(), OAM.end(), 0);
    std::fill(PaletteMemory.begin(), PaletteMemory.end(), 0xFF);

    if (cartridgeTypes[cartridgeType].find("RAM+BATTERY") != std::string::npos) {
        // look for a .sav file of valid size
        std::string saveName = romPath.substr(romPath.find_last_of('/') + 1, romPath.length());
        saveName.erase(saveName.find_last_of('.'));
        saveName.append(".sav");
        std::vector<u8> saveBuffer;
        if (!loadFile(saveName, FileType::SRAM, saveBuffer)) {
            Log(I, "Could not open .sav file %s. Continuing without it.\n", saveName.c_str());
        } else {
            std::copy_n(saveBuffer.begin(), RAM.size(), RAM.begin());
            Log(I, "Successfully loaded SRAM from .sav file %s\n", saveName.c_str());
        }
    }

    VramDma.reset();
    DMACounter = 0, GDMACounter = 0, HDMACounter = 0;
    printCartridgeInfo(buffer);
    return true;
}

bool MMU::loadFile(std::string& path, FileType fileType, std::vector<u8>& buffer) {
    std::ifstream file(path);
    if (!file || !file.good()) {
        Log(W, "Failed to open file %s\n", path.c_str());
        return false;
    }
    file.seekg(0, std::ifstream::end);
    long length = file.tellg();
    file.seekg(0, std::ifstream::beg);

    if (length == -1 || length == 0x7FFFFFFFFFFFFFFF) {
        Log(W, "Failed to load file %s\n", path.c_str());
        return false;
    }

    buffer.resize(length);
    file.read((char *) buffer.data(), length);

    switch (fileType) {
        case FileType::BIOS:
            if ((length == BIOS_SIZE_DMG && cpu->gbMode == DMG) || (length == BIOS_SIZE_CGB && cpu->gbMode == CGB))
                return true;
            Log(W, "Invalid BootROM size: %li. Starting ROM without BIOS.\n", length);
            return false;
        case FileType::ROM:
            // check type id and underlying value
            if (ROMSizeTypes.count(buffer[0x148]) && ROMSizeTypes[buffer[0x148]] == length) return true;
            Log(W, "Cartridge type %d with size %li is invalid\n", buffer[0x148], length);
            return false;
        case FileType::SRAM:
            if (buffer.size() < 517) {
                Log(W, "File %s is too small to be a valid .sav file\n", path.c_str());
                return false;
            }
            std::string header = std::string(&buffer[0], &buffer[0] + 4);
            if (header != "PHOS") {
                Log(W, "Invalid header of .sav file. Expected 'PHOS' but read '%s'\n", header.c_str());
                return false;
            }
            size_t offset;
            if (buffer[4] == 1) {
                offset = 13;
                if (cartridgeTypes[ROM_0[0x147]].find("MBC3") == std::string::npos) {
                    Log(W, "Cartridge type and .sav file type do not match\n");
                    return false;
                }
                long rtc = *reinterpret_cast<long*>(&buffer[5]);
                dynamic_cast<MBC3*>(mbc.get())->latchedTime = rtc;
                dynamic_cast<MBC3*>(mbc.get())->latchClockData();
            } else {
                offset = 5;
            }
            buffer.erase(buffer.begin(), buffer.begin() + offset);

            // RAM is already resized at this point so just compare with that
            if (buffer.size() != RAM.size()) {
                Log(W, "Invalid size of .sav file. Should be %li but detected %li\n", RAM.size(), buffer.size());
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
            if (cpu->gbMode == DMG) return VRAM[address - 0x8000];
            else if (cpu->gbMode == CGB) return VRAM[(address - 0x8000) + VRAMBankPtr * VRAM_BANK_SIZE];
        case 0xA000:
        case 0xB000:
            if (RAM.empty()) return 0xFF;
            return mbc->readRAMByte(address - 0xA000);
        case 0xC000:
        case 0xD000:
            if (cpu->gbMode == DMG) {
                return WRAM[address - 0xC000];
            } else if (cpu->gbMode == CGB) {
                u16 relAddress = address - 0xC000;
                if (relAddress <= 0x0FFF) return WRAM[relAddress];
                else return WRAM[relAddress + WRAMBankPtr * WRAM_BANK_SIZE];
            }
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
                if (address == 0xFF69) {
                    // BG Palette Data
                    return PaletteMemory[IO[0x68] & 0x3F];
                }
                if (address == 0xFF6B) {
                    // Sprite Palette Data
                    return PaletteMemory[(IO[0x6A] & 0x3F) + 0x40];
                }
                return IO[address - 0xFF00];
            }
            return ZRAM[address - 0xFF80];
        default:
            Log(W, "You should not be here\n");
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
            if (cpu->gbMode == DMG) VRAM[address - 0x8000] = value;
            else if (cpu->gbMode == CGB) VRAM[(address - 0x8000) + VRAMBankPtr * VRAM_BANK_SIZE] = value;
            return;
        case 0xA000:
        case 0xB000:
            if (RAM.empty()) return;
            mbc->writeRAMByte(address - 0xA000, value);
            return;
        case 0xC000:
        case 0xD000:
            if (cpu->gbMode == DMG) {
                WRAM[address - 0xC000] = value;
            } else if (cpu->gbMode == CGB) {
                u16 relAddress = address - 0xC000;
                if (relAddress <= 0x0FFF) WRAM[relAddress] = value;
                else WRAM[relAddress + WRAMBankPtr * WRAM_BANK_SIZE] = value;
            }
            return;
        case 0xF000: {
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
                u16 relAddress = address & 0xFF;
                switch (relAddress) {
                    case 0x04:      // reset divider counter on write
                        IO[relAddress] = 0;
                        break;
                    case 0x40: {    // LCD Control
                        bool wasLCDEnabled = IO[0x40] & LCD_DISPLAY_ENABLE;
                        IO[0x40] = value;
                        if (wasLCDEnabled && !(value & LCD_DISPLAY_ENABLE)) {
                            //assert(gpu->getMode() == VBLANK && "Tried to disable display outside of VBLANK period\n");
                            gpu->setBGColor(0xFF);      // set display to all white
                            // reset some STAT values
                            // TODO: see if this is right
                            // i.e. compare with "https://www.reddit.com/r/EmuDev/comments/6r6gf3/gb_pokemon_gold_spews_unexpected_values_at_mbc/dl5c0ub"
                            IO[LCDC_Y_COORDINATE - 0xFF00] = 153;
                            gpu->modeclock = 456;
                            gpu->setMode(VBLANK);
                        }
                        break; }
                    case 0x41:      // LCDC Stat
                        IO[relAddress] = (value & (u8) 0xF8) | (IO[relAddress] & (u8) 0x07);
                        break;
                    case 0x44:      // LY (line) (Read-only)
                        return;
                    case 0x46: {    // Start DMA transfer
                        u16 source = value << (u16) 8;
                        for (int i=0; i<0xA0; i++) {
                            writeByte(0xFE00 + i, readByte(source + i));
                        }
                        IO[relAddress] = value;
                        cpu->gpu.DMATicks = 648;
                        DMACounter++;
                        break; }
                    case 0x4D:      // Speed Switch (CGB Mode Only)
                        IO[relAddress] = (IO[relAddress] & 0xFE) | (value & 0x01);
                        Log(I, "Changed speed mode\n");
                        break;
                    case 0x4F:      // Select VRAM Bank (CGB Mode Only)
                        if (cpu->gbMode != CGB) return;
                        VRAMBankPtr = value & 0x01;
                        IO[relAddress] = VRAMBankPtr;
                        break;
                    case 0x55: {    // VRAM DMA Transfer (CGB Mode Only)
                        if (cpu->gbMode != CGB) return;
                        VramDma.transferLength = ((value & 0x7F) + 1) * 16;
                        bool stat = isBitSet(value, 7);

                        if (stat) {
                            IO[0x55] = value & 0x7F;
                            if (!VramDma.enabled) {
                                VramDma.enabled = true;
                                HDMACounter++;
                            }
                        } else {
                            if (VramDma.enabled) {
                                IO[0x55] = 0xFF;
                                VramDma.enabled = false;
                            } else {
                                performGDMA();
                                GDMACounter++;
                            }
                        }
                        break; }
                    case 0x56:      // Infrared (CGB Mode Only)
                        assert(cpu->gbMode == CGB);
                        Log(I, "Write to CGB Infrared Communications Port [Unimplemented]\n");
                        break;
                    case 0x69:      // BG Palette Data (CGB Mode Only)
                        PaletteMemory[IO[0x68] & 0x3F] = value;
                        if (isBitSet(IO[0x68], 7)) IO[0x68]++;
                        break;
                    case 0x6B:      // Sprite Palette Data (CGB Mode Only)
                        PaletteMemory[(IO[0x6A] & 0x3F) + 0x40] = value;
                        if (isBitSet(IO[0x6A], 7)) IO[0x6A]++;
                        break;
                    case 0x6C:
                        if (cpu->gbMode != CGB) return;
                        IO[relAddress] = (IO[relAddress] & 0xFE) | (value & 0x01);
                        break;
                    case 0x70:      // Select WRAM Bank (CGB Mode Only)
                        assert(cpu->gbMode == CGB);
                        WRAMBankPtr = value & 0x07;
                        IO[relAddress] = WRAMBankPtr;
                        if (WRAMBankPtr == 0x00) WRAMBankPtr = 1;
                        WRAMBankPtr--;
                        break;
                    case 0x74:
                        if (cpu->gbMode != CGB) return;
                        IO[relAddress] = value;
                        break;
                    case 0x75:
                        IO[relAddress] = value & 0x70;
                        break;
                    default:
                        IO[relAddress] = value;
                }
                return;
            }
            ZRAM[address - 0xFF80] = value;
            return; }
        default:
            return;
    }
}

void MMU::performHDMA() {
    assert(cpu->gbMode == CGB);

    u16 source = (IO[0x51] << 8) | (IO[0x52] & 0xF0);
    u16 dest = (((IO[0x53] & 0x1F) << 8) | (IO[0x54] & 0xF0)) + 0x8000;

    assert((source <= 0x7FFF) || (source >= 0xA000 && source <= 0xDFFF));
    assert(dest >= 0x8000 && dest <= 0x9FFF);

    // copy 16 bytes into VRAM
    for (unsigned i=0; i<16; i++) {
        writeByte(dest + i, readByte(source + i));
    }

    // update the remaining transfer length
    VramDma.transferLength -= 16;
    IO[0x55]--;
    if (IO[0x55] == 0xFF) {
        // HDMA finished
        //assert(IO[0x55] == 0xFF);
        VramDma.enabled = false;
    } else {
        source += 16;
        // check overflow
        if (source == 0x8000) source = 0xA000;
        IO[0x51] = source >> 8;
        IO[0x52] = source & 0xFF;
        dest += 16;
        // check overflow
        if (dest == 0xA000) dest = 0x8000;
        IO[0x53] = dest >> 8;
        IO[0x54] = dest & 0xFF;
    }

    /*
    unsigned transferCycles = 4;
    if (cpu->doubleSpeedMode)
        transferCycles += 64;
    else
        transferCycles += 32;
    cpu->cycles += transferCycles;
     */
}

void MMU::performGDMA() {
    assert(cpu->gbMode == CGB);

    u16 source = (IO[0x51] << 8) | (IO[0x52] & 0xF0);
    u16 dest = (((IO[0x53] & 0x1F) << 8) | (IO[0x54] & 0xF0)) + 0x8000;

    assert((source <= 0x7FFF) || (source >= 0xA000 && source <= 0xDFFF));
    assert(dest >= 0x8000 && dest <= 0x9FFF);

    for (unsigned i=0; i<VramDma.transferLength; i++) {
        writeByte(dest+i, readByte(source+i));
    }

    for (unsigned r=0; r<5; r++) {
        IO[0x51+r] = 0xFF;
    }

    unsigned transferCycles = 4;
    if (cpu->doubleSpeedMode)
        transferCycles += VramDma.transferLength * 64;
    else
        transferCycles += VramDma.transferLength * 32;
    cpu->cycles += transferCycles;
}

void MMU::writeWord(u16 address, u16 value) {
    assert(address + 1 <= 0xFFFF);
    u8 low = value & 0xFF;
    u8 high = (value >> 8) & 0xFF;
    writeByte(address, low);
    writeByte(address + 1, high);
}

void MMU::serialize(serializer &s) {
    s.integer(inBIOS);
    s.integer(runBIOS);
    s.integer(WRAMBankPtr);
    s.integer(VRAMBankPtr);
    s.integer(VramDma.enabled);
    s.enumeration(VramDma.mode);
    s.integer(VramDma.transferLength);
    // values with constant size
    s.array(IO.data(), IO.size());
    s.array(ZRAM.data(), ZRAM.size());
    s.array(OAM.data(), OAM.size());
    s.array(PaletteMemory.data(), PaletteMemory.size());
    // values with GB Mode dependent size
    s.array(WRAM.data(), WRAM.size());
    s.array(VRAM.data(), VRAM.size());
    // values with MBC dependent size
    if (!RAM.empty())
        s.array(RAM.data(), RAM.size());
    mbc->serialize(s);
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
    cartridgeTypes[0x05] = "MBC2+RAM";          // MBC2 has build-in RAM
    cartridgeTypes[0x06] = "MBC2+RAM+BATTERY";  // same here
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
    RAMSizeTypes[0x04] = 131072;
    RAMSizeTypes[0x05] = 65536;
}

void MMU::printCartridgeInfo(std::vector<u8>& buffer) {
    cartridgeTitle = std::string(&buffer[0x134], &buffer[0x134] + 0xF);
    u8 cartridgeType = buffer[0x147];

    LogRaw(I, "\n");
    Log(I, "Game Title:          %s\n", cartridgeTitle.c_str());
    Log(I, "Cartridge Type:      0x%02X (%s)\n", cartridgeType, cartridgeTypes[cartridgeType].c_str());
    Log(I, "ROM Size Type:       0x%02X (%d Byte)\n", buffer[0x148], ROMSizeTypes[buffer[0x148]]);
    Log(I, "RAM Size Type:       0x%02X (%d Byte)", buffer[0x149], RAMSizeTypes[buffer[0x149]]);
    if (cartridgeType == 0x05 || cartridgeType == 0x06) LogRaw(I, " -- (%li Byte MBC2 internal)\n", RAM.size());
    else LogRaw(I, "\n");
    Log(I, "Destination Code:    %d", buffer[0x14A]);
    buffer[0x14A] ? LogRaw(I, " (Non-Japanese)\n") : LogRaw(I, " (Japanese)\n");
    u8 licenceCode = buffer[0x14B];
    if (licenceCode == 0x33) {
        std::string newCode = std::string(&buffer[0x144], &buffer[0x144] + 1);
        Log(I, "Licensee Code (New): %s\n", newCode.c_str());
    } else {
        Log(I, "Licensee Code (Old): 0x%02X\n", licenceCode);
    }
    Log(I, "Game Version:        %d\n", buffer[0x14C]);

    u8 headerCRC = buffer[0x14D];
    Log(I, "Header Checksum:     0x%02X", headerCRC);
    u8 x = 0;
    for (int i=0x134; i<=0x14C; i++) x = x - buffer[i] -1;
    if (x != headerCRC) {
        LogRaw(I, "   [INVALID - actual checksum is 0x%02X, a real Gameboy would halt execution]\n", x);
    } else {
        LogRaw(I, "   [VALID]\n");
    }

    u16 globalCRC = (buffer[0x14E] << 8) | buffer[0x14F];
    Log(I, "Global Checksum:     0x%04X", globalCRC);
    u16 g = 0;
    for (u8 i : buffer) g += i;
    g -= buffer[0x14E];
    g -= buffer[0x14F];
    if (g != globalCRC) {
        LogRaw(I, " [INVALID - actual checksum is 0x%04X, but a real Gameboy would not care]\n", g);
    } else {
        LogRaw(I, " [VALID]\n");
    }
    LogRaw(I, "\n");
}
