#include "catch.hpp"

#include "CPU.h"
#include "CPU.cpp"

TEST_CASE("Register endianness test") {
    CPU cpu;
    cpu.regs.af = 1;
    REQUIRE(cpu.regs.a == 0);
    REQUIRE(cpu.regs.f == 1);
    cpu.regs.bc = 0x8000;
    REQUIRE(cpu.regs.b == 0x80);
    REQUIRE(cpu.regs.c == 0);
    cpu.regs.de = 0xFFFF;
    REQUIRE(cpu.regs.d == 0xFF);
    REQUIRE(cpu.regs.e == 0xFF);
    cpu.regs.h = 0x7F;
    cpu.regs.l = 0xFF;
    REQUIRE(cpu.regs.hl == 0x7FFF);
    cpu.regs.hl = 0;
    REQUIRE(cpu.regs.hl == 0);
}

