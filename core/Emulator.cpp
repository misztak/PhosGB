#include "Emulator.h"

Emulator::Emulator(): isHalted(false), isDead(false) {}

bool Emulator::load(std::string& romPath) {
    bool success = cpu.init(romPath);
    if (success) {
        isHalted = false;
        isDead = false;
        currentFilePath = romPath;
        if (std::find(recentFiles.begin(), recentFiles.end(), romPath) == recentFiles.end()) {
            recentFiles.push_back(romPath);
        }
        currentFile = currentFilePath.substr(currentFilePath.find_last_of('/') + 1);
        currentFile.erase(currentFile.find_last_of('.'));
    }
    return success;
}

u32 Emulator::tick() {
    return cpu.tick();
}

void Emulator::toggle() {
    if (isDead) {
        return;
    }
    isHalted = !isHalted;
    if (isHalted) {
        Log(I, "Stopped execution\n");
    } else {
        Log(I, "Resumed execution\n");
    }
}

void Emulator::shutdown() {
    u8 cartridgeType = cpu.mmu.ROM_0[0x147];
    // TODO: move this somewhere else
    if (cpu.mmu.cartridgeTypes[cartridgeType].find("RAM+BATTERY") != std::string::npos) {
        std::string saveName = currentFile + ".sav";
        std::ofstream outfile(saveName, std::ios::out | std::ios::binary);
        outfile.write("PHOS", 4);
        u8 fileType;
        if (cpu.mmu.cartridgeTypes[cartridgeType].find("MBC3") != std::string::npos) {
            fileType = 1;
            outfile.write(reinterpret_cast<char *>(&fileType), 1);
            long rtc = dynamic_cast<MBC3*>(cpu.mmu.mbc.get())->latchedTime;
            outfile.write(reinterpret_cast<char *>(&rtc), 8);
        } else {
            fileType = 0;
            outfile.write(reinterpret_cast<char *>(&fileType), 1);
        }
        outfile.write((char *) cpu.mmu.RAM.data(), cpu.mmu.RAM.size());
        Log(I, "Saved RAM state in file %s\n", saveName.c_str());
    }
}

u8* Emulator::getDisplayState() {
    return cpu.gpu.getDisplayState();
}

bool Emulator::hitVBlank() {
    if (cpu.gpu.hitVBlank) {
        cpu.gpu.hitVBlank = false;
        return true;
    } else {
        return false;
    }
}

void Emulator::handleInputDown(u8 key) {
    cpu.handleInputDown(key);
}

void Emulator::handleInputUp(u8 key) {
    cpu.handleInputUp(key);
}

void Emulator::saveState() {
    std::string saveName = currentFile + "_Quicksave.state";
//    auto dateTime = std::time(nullptr);
//    auto now = std::localtime(&dateTime);
//    std::ostringstream oss;
//    oss << '[' << (now->tm_year + 1900) << '-' << (now->tm_mon + 1) << '-' << now->tm_mday << "--" << now->tm_hour << ':' << now->tm_min << ':' << now->tm_sec << ']';
//    saveName.append(oss.str());

    std::ofstream outfile(saveName, std::ios::out | std::ios::binary);
    outfile.write("PHOS-STATE ", 11);
    // start with cartridge info
    outfile.write(cpu.mmu.cartridgeTitle.c_str(), cpu.mmu.cartridgeTitle.size());
    outfile.write(WRITE_A(cpu.mmu.ROM_0, 0x147), 3);
    // dump CPU values
    outfile.write(WRITE_V(cpu.r.af), 2);
    outfile.write(WRITE_V(cpu.r.bc), 2);
    outfile.write(WRITE_V(cpu.r.de), 2);
    outfile.write(WRITE_V(cpu.r.hl), 2);
    outfile.write(WRITE_V(cpu.r.pc), 2);
    outfile.write(WRITE_V(cpu.r.sp), 2);
    outfile.write(WRITE_V(cpu.halted), sizeof(bool));
    outfile.write(WRITE_V(cpu.timerCounter), 4);
    outfile.write(WRITE_V(cpu.dividerCounter), 4);
    // dump subsystems
    cpu.joypad.saveState(outfile);
    cpu.gpu.saveState(outfile);
    cpu.apu.reset();
    cpu.mmu.saveState(outfile);

    Log(I, "Saved state in file %s\n", saveName.c_str());
}

bool Emulator::loadState(std::string &path) {
    if (path.substr(path.find_last_of('.')) != ".state") {
        Log(W, "Invalid save state file suffix\n");
        return false;
    }

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

    std::vector<u8> buffer;
    buffer.resize(length);
    file.read((char *) buffer.data(), length);

    // check if state file matches with the current cartridge
    std::string header = std::string(&buffer[0], &buffer[0] + 11);
    if (header != "PHOS-STATE ") {
        Log(W, "Invalid header of .state file. Expected 'PHOS-STATE ' but read '%s'\n", header.c_str());
        return false;
    }
    std::string cartName = std::string(&buffer[11], &buffer[11] + 15);
    u8 cartType = READ_U8(&buffer[0x1A]);
    u8 romType = READ_U8(&buffer[0x1B]);
    u8 ramType = READ_U8(&buffer[0x1C]);
    if (cartName != cpu.mmu.cartridgeTitle || cartType != cpu.mmu.ROM_0[0x147] || romType != cpu.mmu.ROM_0[0x148] ||
    ramType != cpu.mmu.ROM_0[0x149]) {
        Log(W, "Save state file does not match with current cartridge\n");
        return false;
    }

    // initialize saved state
    cpu.r.af = READ_U16(&buffer[0x1D]);
    cpu.r.bc = READ_U16(&buffer[0x1F]);
    cpu.r.de = READ_U16(&buffer[0x21]);
    cpu.r.hl = READ_U16(&buffer[0x23]);
    cpu.r.pc = READ_U16(&buffer[0x25]);
    cpu.r.sp = READ_U16(&buffer[0x27]);
    cpu.halted = READ_BOOL(&buffer[0x29]);
    cpu.timerCounter = READ_S32(&buffer[0x2A]);
    cpu.dividerCounter = READ_S32(&buffer[0x2E]);
    cpu.joypad.loadState(buffer);
    cpu.apu.reset();
    cpu.mmu.loadState(buffer);
    cpu.gpu.loadState(buffer);

    Log(I, "Load state from file %s\n", path.c_str());
    return true;
}
