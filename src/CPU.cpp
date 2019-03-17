#include "CPU.h"

CPU::CPU() {
    regs.af = 0;
    regs.bc = 0;
    regs.de = 0;
    regs.hl = 0;
    regs.sp = 0;
    regs.pc = 0;

    byteRegisterMap[0x0] = &regs.b;
    byteRegisterMap[0x1] = &regs.c;
    byteRegisterMap[0x2] = &regs.d;
    byteRegisterMap[0x3] = &regs.e;
    byteRegisterMap[0x4] = &regs.h;
    byteRegisterMap[0x5] = &regs.l;
    byteRegisterMap[0x6] = &regs.f;
    byteRegisterMap[0x7] = &regs.a;

    shortRegisterMap[0x0] = &regs.bc;
    shortRegisterMap[0x1] = &regs.de;
    shortRegisterMap[0x2] = &regs.hl;
    shortRegisterMap[0x3] = &regs.sp;

    instructions[0x00] = &CPU::NOP;
}

void CPU::reset() {

}

void CPU::setFlag(FLAG flag) {
    regs.f |= flag;
}

void CPU::clearFlag(FLAG flag) {
    regs.f &= ~flag;
}

bool CPU::isFlagSet(FLAG flag) {
    return regs.f &= flag;
}

u32 CPU::NOP(const u8 &opcode) {
    return 4;
}

u32 CPU::LDrr(const u8 &opcode) {
    return 4;
}
