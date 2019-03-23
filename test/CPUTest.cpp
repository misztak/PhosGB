#include "catch.hpp"

#include "CPU.h"
#include "CPU.cpp"

TEST_CASE("Register endianness test") {
    CPU cpu;
    cpu.r.af = 1;
    REQUIRE(cpu.r.a == 0);
    REQUIRE(cpu.r.f == 1);
    cpu.r.bc = 0x8000;
    REQUIRE(cpu.r.b == 0x80);
    REQUIRE(cpu.r.c == 0);
    cpu.r.de = 0xFFFF;
    REQUIRE(cpu.r.d == 0xFF);
    REQUIRE(cpu.r.e == 0xFF);
    cpu.r.h = 0x7F;
    cpu.r.l = 0xFF;
    REQUIRE(cpu.r.hl == 0x7FFF);
    cpu.r.hl = 0;
    REQUIRE(cpu.r.hl == 0);
}

