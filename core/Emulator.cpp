#include "Emulator.h"

Emulator::Emulator(): isHalted(false), isDead(false) {}

bool Emulator::load(std::string& romPath) {
    bool success = cpu.init(romPath);
    if (success) {
        isHalted = false;
        isDead = false;
        currentFile = romPath;
        if (std::find(recentFiles.begin(), recentFiles.end(), romPath) == recentFiles.end()) {
            recentFiles.push_back(romPath);
        }
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
        printf("Stopped execution\n");
    } else {
        printf("Resumed execution\n");
    }
}

void Emulator::kill() {
    isDead = true;
    isHalted = true;
}

void Emulator::shutdown() {
    u8 cartridgeType = cpu.mmu.ROM_0[0x147];
    // TODO: move this somewhere else
    if (cpu.mmu.cartridgeTypes[cartridgeType].find("RAM+BATTERY") != std::string::npos) {
        std::string saveName = currentFile.substr(currentFile.find_last_of('/') + 1, currentFile.size());
        saveName.append(".sav");
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
        printf("Saving RAM state to %s\n", saveName.c_str());
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
    std::string saveName = currentFile.substr(currentFile.find_last_of('/') + 1);
    saveName = saveName.erase(saveName.find_last_of('.'));
    auto dateTime = std::time(nullptr);
    auto now = std::localtime(&dateTime);
    std::ostringstream oss;
    oss << '[' << (now->tm_year + 1900) << '-' << (now->tm_mon + 1) << '-' << now->tm_mday << "--" << now->tm_hour << ':' << now->tm_min << ':' << now->tm_sec << ']';
    saveName.append(oss.str());
    saveName.append(".state");

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
    cpu.apu.saveState(outfile);
    cpu.mmu.saveState(outfile);

    printf("Saved state in file %s\n", saveName.c_str());
}

bool Emulator::loadState(std::string &path) {
    return false;
}
