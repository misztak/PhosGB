#include "Emulator.hpp"

Emulator::Emulator(): isHalted(false), waitingForFile(true) {}

bool Emulator::load(std::string& romPath) {
    bool success = cpu.init(romPath);
    if (success) {
        isHalted = false;
        waitingForFile = false;
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

void Emulator::pause() {
    if (waitingForFile) {
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
    u32 serializerSize = serializeInit();
    serializer s(serializerSize);

    char header[11] = "PHOS-STATE";
    s.array(header);

    serializeAll(s);

    std::string saveName = currentFile + "_Quicksave.state";
    std::ofstream outfile(saveName, std::ios::out | std::ios::binary);
    outfile.write((char*) s.data(), s.size());
    Log(I, "Saved state in file %s\n", saveName.c_str());
}

bool Emulator::loadState(std::string& path) {
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

    if (length == -1 || length > 8388608) {
        Log(W, "Failed to load file %s\n", path.c_str());
        return false;
    }

    u8* buf = new u8[length];
    file.read((char *) buf, length);

    u32 serializerSize = serializeInit();
    if (serializerSize != length) {
        Log(W, "Size of save state file does not match serializer size\n");
        return false;
    }
    serializer s(buf, length);
    delete[] buf;

    char header[11] = {0};
    s.array(header);
    if (std::string(header) != "PHOS-STATE") {
        Log(W, "Save state file has invalid header\n");
        return false;
    }

    serializeAll(s);

    Log(I, "Load state from file %s\n", path.c_str());
    return true;
}

u32 Emulator::serializeInit() {
    serializer s;

    char header[11] = {0};
    // TODO: add some sort of checksum
    s.array(header);

    serializeAll(s);
    return s.size();
}

void Emulator::serializeAll(serializer& s) {
    cpu.serialize(s);
    cpu.joypad.serialize(s);
    cpu.gpu.serialize(s);
    cpu.apu.reset();
    cpu.mmu.serialize(s);
}

std::string Emulator::currentDateTime() {
    auto dateTime = std::time(nullptr);
    auto now = std::localtime(&dateTime);
    std::ostringstream oss;
    oss << '[' << (now->tm_year + 1900) << '-' << (now->tm_mon + 1) << '-' << now->tm_mday << "--" << now->tm_hour << '-' << now->tm_min << '-' << now->tm_sec << ']';
    return oss.str();
}
