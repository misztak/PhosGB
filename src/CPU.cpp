#include "CPU.h"

CPU::CPU() {
    regs.af = 0;
    regs.bc = 0;
    regs.de = 0;
    regs.hl = 0;
    regs.sp = 0;
    regs.pc = 0;
}