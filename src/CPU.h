#ifndef PHOS_CPU_H
#define PHOS_CPU_H

#include <map>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

// bitmasks for flags stored in lower 8bit of AF register
enum FLAG { ZERO = 0x80, ADD_SUB = 0x40, HALF_CARRY = 0x20, CARRY = 0x10};

// first four registers can be accessed as one 16bit or two separate 8bit registers
struct registers {
    union {
        struct {
            u8 f;
            u8 a;
        };
        u16 af;
    };

    union {
        struct {
            u8 c;
            u8 b;
        };
        u16 bc;
    };

    union {
        struct {
            u8 e;
            u8 d;
        };
        u16 de;
    };

    union {
        struct {
            u8 l;
            u8 h;
        };
        u16 hl;
    };

    u16 sp;
    u16 pc;
};

class CPU {
public:
    struct registers regs;
public:
    CPU();
    void reset();
private:
    u8* byteRegisterMap[8];
    u16* shortRegisterMap[4];

    typedef u32 (CPU::*Instruction)(const u8& opcode);
    Instruction instructions[256];
    Instruction instructionsCB[256];
private:
    void setFlag(FLAG flag);
    void clearFlag(FLAG flag);
    bool isFlagSet(FLAG flag);

    u32 NOP(const u8& opcode);
    u32 LDrr(const u8& opcode);
};

#endif //PHOS_CPU_H
