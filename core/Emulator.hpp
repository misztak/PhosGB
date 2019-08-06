#ifndef PHOS_EMULATOR_HPP
#define PHOS_EMULATOR_HPP

#include <sstream>

#include "Common.hpp"
#include "CPU.hpp"

class Emulator {
public:
    Emulator();
    bool load(std::string& romPath);
    void pause();
    void shutdown();
    u32 tick();
    u8* getDisplayState();
    bool hitVBlank();
    void handleInputDown(u8 key);
    void handleInputUp(u8 key);
    void saveState();
    bool loadState(std::string& path);
    std::string currentDateTime();
public:
    bool isHalted;
    CPU cpu;
    std::string currentFile;
    std::string currentFilePath;
    std::vector<std::string> recentFiles;
private:
    u32 serializeInit();
    void serializeAll(serializer& s);
private:
    bool waitingForFile;
};

#endif //PHOS_EMULATOR_HPP
