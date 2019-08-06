#include <chrono>

#include "catch.hpp"
#include "Emulator.hpp"

Emulator emu;

void runForDuration(int timeInSeconds = 5) {
    auto duration = std::chrono::system_clock::now() + std::chrono::seconds(timeInSeconds);
    while (std::chrono::system_clock::now() < duration) {
        int ticks = 0;
        while (ticks < emu.cpu.ticksPerFrame) {
            ticks += emu.tick();
        }
    }
}

TEST_CASE("CPU INSTRUCTION TEST") {
    emu.cpu.headless = true;
    std::string filePath = "../gb/blargg/cpu_instrs.gb";
    REQUIRE(emu.load(filePath));

    runForDuration();

    bool result =   emu.cpu.mmu.ZRAM[0x40] == 0x4C &&
                    emu.cpu.mmu.ZRAM[0x41] == 0xD2 &&
                    emu.cpu.mmu.ZRAM[0x42] == 0x33 &&
                    emu.cpu.mmu.ZRAM[0x43] == 0xFE;
    REQUIRE(result);
}

TEST_CASE("CPU INSTRUCTION TIMING") {
    emu.cpu.headless = true;
    std::string filePath = "../gb/blargg/instr_timing.gb";
    REQUIRE(emu.load(filePath));

    runForDuration();

    bool result =   emu.cpu.mmu.ZRAM[0x00] == 0x8E &&
                    emu.cpu.mmu.ZRAM[0x01] == 0xEB &&
                    emu.cpu.mmu.ZRAM[0x02] == 0x89 &&
                    emu.cpu.mmu.ZRAM[0x03] == 0x9B;
    REQUIRE(result);
}