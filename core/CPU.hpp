#ifndef PHOS_CPU_HPP
#define PHOS_CPU_HPP

#include "Common.hpp"
#include "MMU.hpp"
#include "GPU.hpp"
#include "Joypad.hpp"
#include "APU.hpp"

enum GB_MODE { DMG, CGB };

// bitmasks for flags stored in lower 8bit of AF register
enum FLAG { ZERO = 0x80, ADD_SUB = 0x40, HALF_CARRY = 0x20, CARRY = 0x10 };

// first four registers can be accessed as one 16bit or two separate 8bit registers
typedef struct Registers {
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
    u8 ime;

    Registers(u16 af, u16 bc, u16 de, u16 hl, u16 sp, u16 pc, u8 ime) :
        af(af), bc(bc), de(de), hl(hl), sp(sp), pc(pc), ime(ime) {};
} Registers;

class CPU {
public:
    Registers r;
    MMU mmu;
    GPU gpu;
    Joypad joypad;
    APU apu;

    GB_MODE gbMode;

    u32 cycles;
    int ticksPerFrame;
    bool halted;
    int timerCounter;
    int dividerCounter;

    bool headless;
    bool runCGBinDMGMode;
    bool doubleSpeedMode;
public:
    CPU();
    bool init(std::string& romPath);
    void reset();
    u32 tick();
    void requestInterrupt(u8 interrupt);
    void handleInputDown(u8 key);
    void handleInputUp(u8 key);

    u8 readByte(u16 address);
    void writeByte(u16 address, u8 value);
    u16 readWord(u16 address);
    void writeWord(u16 address, u16 value);

    void serialize(phos::serializer& s);
private:
    u8*  byteRegisterMap[8]  = {nullptr};
    u16* shortRegisterMap[4] = {nullptr};

    typedef u32 (CPU::*Instruction)(const u8& opcode);
    Instruction instructions[256]   = {nullptr};
    Instruction instructionsCB[256] = {nullptr};
private:
    void setFlag(FLAG flag);
    void clearFlag(FLAG flag);
    bool isFlagSet(FLAG flag);

    void checkInterrupts();
    void updateTimer(u32 ticks);
    void setTimerFreq();

    void pushByte(u8 value);
    void pushWord(u16 value);
    u8 popByte();
    u16 popWord();
    u8* byteRegister(u8 opcode);
    u16* wordRegister(u8 opcode);

    void checkHalfCarry(u8 reg);
    void checkCarry(u8 reg);

    // Z80 Instructions //

    u32 LD_r_n(const u8& opcode);       // 0x06, 0x0E, 0x16, 0x1E, 0x26, 0x2E, 0x3E
    u32 LD_r_r(const u8& opcode);       // 0x7F, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D
                                        // 0x40, 0x41, 0x42, 0x43, 0x44, 0x45
                                        // 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D
                                        // 0x50, 0x51, 0x52, 0x53, 0x54, 0x55
                                        // 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D
                                        // 0x60, 0x61, 0x62, 0x63, 0x64, 0x65
                                        // 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D
                                        // 0x47, 0x4F, 0x57, 0x5F, 0x67, 0x6F
    u32 LD_r_HL(const u8& opcode);      // 0x7E, 0x46, 0x4E, 0x56, 0x5E, 0x66, 0x6E
    u32 LD_HL_r(const u8& opcode);      // 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x77
    u32 LD_HL_n(const u8& opcode);      // 0x36
    u32 LD_A_BC(const u8& opcode);      // 0x0A
    u32 LD_A_DE(const u8& opcode);      // 0x1A
    u32 LD_A_nn(const u8& opcode);      // 0xFA
    u32 LD_BC_A(const u8& opcode);      // 0x02
    u32 LD_DE_A(const u8& opcode);      // 0x12
    u32 LD_nn_A(const u8& opcode);      // 0xEA
    u32 LD_A_Cff00(const u8& opcode);   // 0xF2
    u32 LD_Cff00_A(const u8& opcode);   // 0xE2
    u32 LDD_A_HL(const u8& opcode);     // 0x3A
    u32 LDD_HL_A(const u8& opcode);     // 0x32
    u32 LDI_A_HL(const u8& opcode);     // 0x2A
    u32 LDI_HL_A(const u8& opcode);     // 0x22
    u32 LD_nff00_A(const u8& opcode);   // 0xE0
    u32 LD_A_nff00(const u8& opcode);   // 0xF0

    u32 LD_r2_nn(const u8& opcode);     // 0x01, 0x11, 0x21, 0x31
    u32 LD_SP_HL(const u8& opcode);     // 0xF9
    u32 LD_HL_SPn(const u8& opcode);    // 0xF8
    u32 LD_nn_SP(const u8& opcode);     // 0x08
    u32 PUSH_r2(const u8& opcode);      // 0xF5, 0xC5, 0xD5, 0xE5
    u32 POP_r2(const u8& opcode);       // 0xF1, 0xC1, 0xD1, 0xE1

    u32 ADD_A_r(const u8& opcode);      // 0x87, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85
    u32 ADD_A_HL(const u8& opcode);     // 0x86
    u32 ADD_A_n(const u8& opcode);      // 0xC6
    u32 ADC_A_r(const u8& opcode);      // 0x8F, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D
    u32 ADC_A_HL(const u8& opcode);     // 0x8E
    u32 ADC_A_n(const u8& opcode);      // 0xCE
    u32 SUB_A_r(const u8& opcode);      // 0x97, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95
    u32 SUB_A_HL(const u8& opcode);     // 0x96
    u32 SUB_A_n(const u8& opcode);      // 0xD6
    u32 SBC_A_r(const u8& opcode);      // 0x9F, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D
    u32 SBC_A_HL(const u8& opcode);     // 0x9E
    u32 SBC_A_n(const u8& opcode);      // 0xDE
    u32 AND_A_r(const u8& opcode);      // 0xA7, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5
    u32 AND_A_HL(const u8& opcode);     // 0xA6
    u32 AND_A_n(const u8& opcode);      // 0xE6
    u32 OR_A_r(const u8& opcode);       // 0xB7, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5
    u32 OR_A_HL(const u8& opcode);      // 0xB6
    u32 OR_A_n(const u8& opcode);       // 0xF6
    u32 XOR_A_r(const u8& opcode);      // 0xAF, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD
    u32 XOR_A_HL(const u8& opcode);     // 0xAE
    u32 XOR_A_n(const u8& opcode);      // 0xEE
    u32 CP_A_r(const u8& opcode);       // 0xBF, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD
    u32 CP_A_HL(const u8& opcode);      // 0xBE
    u32 CP_A_n(const u8& opcode);       // 0xFE
    u32 INC_r(const u8& opcode);        // 0x3C, 0x04, 0x0C, 0x14, 0x1C, 0x24, 0x2C
    u32 INC_HL(const u8& opcode);       // 0x34
    u32 DEC_r(const u8& opcode);        // 0x3D, 0x05, 0x0D, 0x15, 0x1D, 0x25, 0x2D
    u32 DEC_HL(const u8& opcode);       // 0x35

    u32 ADD_HL_r2(const u8& opcode);    // 0x09, 0x19, 0x29, 0x39
    u32 ADD_SP_sn(const u8& opcode);    // 0xE8
    u32 INC_r2(const u8& opcode);       // 0x03, 0x13, 0x23, 0x33
    u32 DEC_r2(const u8& opcode);       // 0x0B, 0x1B, 0x2B, 0x3B

    u32 DAA(const u8& opcode);          // 0x27
    u32 CPL(const u8& opcode);          // 0x2F
    u32 CCF(const u8& opcode);          // 0x3F
    u32 SCF(const u8& opcode);          // 0x37
    u32 NOP(const u8& opcode);          // 0x00
    u32 HALT(const u8& opcode);         // 0x76
    u32 STOP(const u8& opcode);         // 0x10 0x00
    u32 DI(const u8& opcode);           // 0xF3
    u32 EI(const u8& opcode);           // 0xFB

    u32 RLCA(const u8& opcode);         // 0x07
    u32 RLA(const u8& opcode);          // 0x17
    u32 RRCA(const u8& opcode);         // 0x0F;
    u32 RRA(const u8& opcode);          // 0x1F

    u32 JP_nn(const u8& opcode);        // 0xC3
    u32 JP_cc_nn(const u8& opcode);     // 0xC2, 0xCA, 0xD2, 0xDA
    u32 JP_HL(const u8& opcode);        // 0xE9
    u32 JR_sn(const u8& opcode);        // 0x18
    u32 JR_cc_sn(const u8& opcode);     // 0x20, 0x28, 0x30, 0x38

    u32 CALL_nn(const u8& opcode);      // 0xCD
    u32 CALL_cc_nn(const u8& opcode);   // 0xC4, 0xCC, 0xD4, 0xDC

    u32 RST_n(const u8& opcode);        // 0xC7, 0xCF, 0xD7, 0xDF, 0xE7, 0xEF, 0xF7, 0xFF
    u32 RET(const u8& opcode);          // 0xC9
    u32 RET_cc(const u8& opcode);       // 0xC0, 0xC8, 0xD0, 0xD8
    u32 RETI(const u8& opcode);         // 0xD9

    // CB Instructions //

    u32 RLC_r(const u8& opcode);        // CB: 0x07, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05
    u32 RLC_HL(const u8& opcode);       // CB: 0x06
    u32 RL_r(const u8& opcode);         // CB: 0x17, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15
    u32 RL_HL(const u8& opcode);        // CB: 0x16
    u32 RRC_r(const u8& opcode);        // CB: 0x0F, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D
    u32 RRC_HL(const u8& opcode);       // CB: 0x0E
    u32 RR_r(const u8& opcode);         // CB: 0x1F, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D
    u32 RR_HL(const u8& opcode);        // CB: 0x1E
    u32 SLA_r(const u8& opcode);        // CB: 0x27, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25
    u32 SLA_HL(const u8& opcode);       // CB: 0x26
    u32 SRA_r(const u8& opcode);        // CB: 0x2F, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D
    u32 SRA_HL(const u8& opcode);       // CB: 0x2E
    u32 SRL_r(const u8& opcode);        // CB: 0x3F, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D
    u32 SRL_HL(const u8& opcode);       // CB: 0x3E

    u32 BIT_b_r(const u8& opcode);      // CB: 0x40 --> 0x7F
    u32 BIT_b_HL(const u8& opcode);     // CB: 0x46
    u32 SET_b_r(const u8& opcode);      // CB: 0xC0 --> 0xFF
    u32 SET_b_HL(const u8& opcode);     // CB: 0xC6
    u32 RES_b_r(const u8& opcode);      // CB: 0x80 --> 0xBF
    u32 RES_b_HL(const u8& opcode);     // CB: 0x86

    u32 SWAP_r(const u8& opcode);       // CB: 0x37, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35
    u32 SWAP_HL(const u8& opcode);      // CB: 0x36

};

#endif //PHOS_CPU_HPP
