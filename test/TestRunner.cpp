#include <chrono>

#include "catch.hpp"
#include "Emulator.h"

TEST_CASE("CPU INSTRUCTION TEST") {
    Emulator emulator;
    emulator.cpu.headless = true;
    std::string filePath = "../gb/blargg/cpu_instrs.gb";
    REQUIRE(emulator.load(filePath));

    auto duration = std::chrono::system_clock::now() + std::chrono::seconds(5);
    while (std::chrono::system_clock::now() < duration) {
        int ticks = 0;
        while (ticks < 70224) {
            ticks += emulator.tick();
        }
    }

    bool result =   emulator.cpu.mmu.ZRAM[0x40] == 0x4C &&
                    emulator.cpu.mmu.ZRAM[0x41] == 0xD2 &&
                    emulator.cpu.mmu.ZRAM[0x42] == 0x33 &&
                    emulator.cpu.mmu.ZRAM[0x43] == 0xFE;
    REQUIRE(result);
}