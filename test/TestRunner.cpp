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

TEST_CASE("CPU MEMORY TIMING 1") {
    emu.cpu.headless = true;
    std::string filePath = "../gb/blargg/mem_timing.gb";
    REQUIRE(emu.load(filePath));

    runForDuration();

    bool result =   emu.cpu.mmu.ZRAM[0x40] == 0x5C &&
                    emu.cpu.mmu.ZRAM[0x41] == 0xE9 &&
                    emu.cpu.mmu.ZRAM[0x42] == 0x81 &&
                    emu.cpu.mmu.ZRAM[0x43] == 0xA1;

    REQUIRE(result);
}

TEST_CASE("CPU MEMORY TIMING 2") {
    emu.cpu.headless = true;
    std::string filePath = "../gb/blargg/mem_timing-2.gb";
    REQUIRE(emu.load(filePath));

    runForDuration();

    bool result =   emu.cpu.mmu.ZRAM[0x00] == 0x05 &&
                    emu.cpu.mmu.ZRAM[0x01] == 0xB7 &&
                    emu.cpu.mmu.ZRAM[0x02] == 0x37 &&
                    emu.cpu.mmu.ZRAM[0x03] == 0x3E &&
                    emu.cpu.mmu.ZRAM[0x04] == 0x00 &&
                    emu.cpu.mmu.ZRAM[0x05] == 0x03;
    REQUIRE(result);
}
