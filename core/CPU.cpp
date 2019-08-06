#include "CPU.hpp"

CPU::CPU():
    r(0, 0, 0, 0, 0, 0, 0),
    gpu(this, &mmu),
    joypad(this),
    apu(this),
    gbMode(DMG),
    cycles(0),
    ticksPerFrame(70224),
    halted(false),
    timerCounter(1024),
    dividerCounter(0),
    headless(false),
    runCGBinDMGMode(false),
    doubleSpeedMode(false)
    {
    mmu.cpu = this;
    mmu.gpu = &gpu;

    byteRegisterMap[0x0] = &r.b;
    byteRegisterMap[0x1] = &r.c;
    byteRegisterMap[0x2] = &r.d;
    byteRegisterMap[0x3] = &r.e;
    byteRegisterMap[0x4] = &r.h;
    byteRegisterMap[0x5] = &r.l;
    byteRegisterMap[0x6] = &r.f;
    byteRegisterMap[0x7] = &r.a;

    shortRegisterMap[0x0] = &r.bc;
    shortRegisterMap[0x1] = &r.de;
    shortRegisterMap[0x2] = &r.hl;
    shortRegisterMap[0x3] = &r.sp;

    // Z80 Instructions //

    instructions[0x06] = &CPU::LD_r_n;
    instructions[0x0E] = &CPU::LD_r_n;
    instructions[0x16] = &CPU::LD_r_n;
    instructions[0x1E] = &CPU::LD_r_n;
    instructions[0x26] = &CPU::LD_r_n;
    instructions[0x2E] = &CPU::LD_r_n;
    instructions[0x3E] = &CPU::LD_r_n;

    instructions[0x7F] = &CPU::LD_r_r;
    instructions[0x78] = &CPU::LD_r_r;
    instructions[0x79] = &CPU::LD_r_r;
    instructions[0x7A] = &CPU::LD_r_r;
    instructions[0x7B] = &CPU::LD_r_r;
    instructions[0x7C] = &CPU::LD_r_r;
    instructions[0x7D] = &CPU::LD_r_r;
    instructions[0x40] = &CPU::LD_r_r;
    instructions[0x41] = &CPU::LD_r_r;
    instructions[0x42] = &CPU::LD_r_r;
    instructions[0x43] = &CPU::LD_r_r;
    instructions[0x44] = &CPU::LD_r_r;
    instructions[0x45] = &CPU::LD_r_r;
    instructions[0x48] = &CPU::LD_r_r;
    instructions[0x49] = &CPU::LD_r_r;
    instructions[0x4A] = &CPU::LD_r_r;
    instructions[0x4B] = &CPU::LD_r_r;
    instructions[0x4C] = &CPU::LD_r_r;
    instructions[0x4D] = &CPU::LD_r_r;
    instructions[0x50] = &CPU::LD_r_r;
    instructions[0x51] = &CPU::LD_r_r;
    instructions[0x52] = &CPU::LD_r_r;
    instructions[0x53] = &CPU::LD_r_r;
    instructions[0x54] = &CPU::LD_r_r;
    instructions[0x55] = &CPU::LD_r_r;
    instructions[0x58] = &CPU::LD_r_r;
    instructions[0x59] = &CPU::LD_r_r;
    instructions[0x5A] = &CPU::LD_r_r;
    instructions[0x5B] = &CPU::LD_r_r;
    instructions[0x5C] = &CPU::LD_r_r;
    instructions[0x5D] = &CPU::LD_r_r;
    instructions[0x60] = &CPU::LD_r_r;
    instructions[0x61] = &CPU::LD_r_r;
    instructions[0x62] = &CPU::LD_r_r;
    instructions[0x63] = &CPU::LD_r_r;
    instructions[0x64] = &CPU::LD_r_r;
    instructions[0x65] = &CPU::LD_r_r;
    instructions[0x68] = &CPU::LD_r_r;
    instructions[0x69] = &CPU::LD_r_r;
    instructions[0x6A] = &CPU::LD_r_r;
    instructions[0x6B] = &CPU::LD_r_r;
    instructions[0x6C] = &CPU::LD_r_r;
    instructions[0x6D] = &CPU::LD_r_r;
    instructions[0x47] = &CPU::LD_r_r;
    instructions[0x4F] = &CPU::LD_r_r;
    instructions[0x57] = &CPU::LD_r_r;
    instructions[0x5F] = &CPU::LD_r_r;
    instructions[0x67] = &CPU::LD_r_r;
    instructions[0x6F] = &CPU::LD_r_r;

    instructions[0x7E] = &CPU::LD_r_HL;
    instructions[0x46] = &CPU::LD_r_HL;
    instructions[0x4E] = &CPU::LD_r_HL;
    instructions[0x56] = &CPU::LD_r_HL;
    instructions[0x5E] = &CPU::LD_r_HL;
    instructions[0x66] = &CPU::LD_r_HL;
    instructions[0x6E] = &CPU::LD_r_HL;

    instructions[0x70] = &CPU::LD_HL_r;
    instructions[0x71] = &CPU::LD_HL_r;
    instructions[0x72] = &CPU::LD_HL_r;
    instructions[0x73] = &CPU::LD_HL_r;
    instructions[0x74] = &CPU::LD_HL_r;
    instructions[0x75] = &CPU::LD_HL_r;
    instructions[0x77] = &CPU::LD_HL_r;

    instructions[0x36] = &CPU::LD_HL_n;
    instructions[0x0A] = &CPU::LD_A_BC;
    instructions[0x1A] = &CPU::LD_A_DE;
    instructions[0xFA] = &CPU::LD_A_nn;
    instructions[0x02] = &CPU::LD_BC_A;
    instructions[0x12] = &CPU::LD_DE_A;
    instructions[0xEA] = &CPU::LD_nn_A;
    instructions[0xF2] = &CPU::LD_A_Cff00;
    instructions[0xE2] = &CPU::LD_Cff00_A;
    instructions[0x3A] = &CPU::LDD_A_HL;
    instructions[0x32] = &CPU::LDD_HL_A;
    instructions[0x2A] = &CPU::LDI_A_HL;
    instructions[0x22] = &CPU::LDI_HL_A;
    instructions[0xE0] = &CPU::LD_nff00_A;
    instructions[0xF0] = &CPU::LD_A_nff00;

    instructions[0x01] = &CPU::LD_r2_nn;
    instructions[0x11] = &CPU::LD_r2_nn;
    instructions[0x21] = &CPU::LD_r2_nn;
    instructions[0x31] = &CPU::LD_r2_nn;

    instructions[0xF9] = &CPU::LD_SP_HL;
    instructions[0xF8] = &CPU::LD_HL_SPn;
    instructions[0x08] = &CPU::LD_nn_SP;

    instructions[0xF5] = &CPU::PUSH_r2;
    instructions[0xC5] = &CPU::PUSH_r2;
    instructions[0xD5] = &CPU::PUSH_r2;
    instructions[0xE5] = &CPU::PUSH_r2;
    instructions[0xF1] = &CPU::POP_r2;
    instructions[0xC1] = &CPU::POP_r2;
    instructions[0xD1] = &CPU::POP_r2;
    instructions[0xE1] = &CPU::POP_r2;

    instructions[0x87] = &CPU::ADD_A_r;
    instructions[0x80] = &CPU::ADD_A_r;
    instructions[0x81] = &CPU::ADD_A_r;
    instructions[0x82] = &CPU::ADD_A_r;
    instructions[0x83] = &CPU::ADD_A_r;
    instructions[0x84] = &CPU::ADD_A_r;
    instructions[0x85] = &CPU::ADD_A_r;
    instructions[0x86] = &CPU::ADD_A_HL;
    instructions[0xC6] = &CPU::ADD_A_n;

    instructions[0x8F] = &CPU::ADC_A_r;
    instructions[0x88] = &CPU::ADC_A_r;
    instructions[0x89] = &CPU::ADC_A_r;
    instructions[0x8A] = &CPU::ADC_A_r;
    instructions[0x8B] = &CPU::ADC_A_r;
    instructions[0x8C] = &CPU::ADC_A_r;
    instructions[0x8D] = &CPU::ADC_A_r;
    instructions[0x8E] = &CPU::ADC_A_HL;
    instructions[0xCE] = &CPU::ADC_A_n;

    instructions[0x97] = &CPU::SUB_A_r;
    instructions[0x90] = &CPU::SUB_A_r;
    instructions[0x91] = &CPU::SUB_A_r;
    instructions[0x92] = &CPU::SUB_A_r;
    instructions[0x93] = &CPU::SUB_A_r;
    instructions[0x94] = &CPU::SUB_A_r;
    instructions[0x95] = &CPU::SUB_A_r;
    instructions[0x96] = &CPU::SUB_A_HL;
    instructions[0xD6] = &CPU::SUB_A_n;

    instructions[0x9F] = &CPU::SBC_A_r;
    instructions[0x98] = &CPU::SBC_A_r;
    instructions[0x99] = &CPU::SBC_A_r;
    instructions[0x9A] = &CPU::SBC_A_r;
    instructions[0x9B] = &CPU::SBC_A_r;
    instructions[0x9C] = &CPU::SBC_A_r;
    instructions[0x9D] = &CPU::SBC_A_r;
    instructions[0x9E] = &CPU::SBC_A_HL;
    instructions[0xDE] = &CPU::SBC_A_n;

    instructions[0xA7] = &CPU::AND_A_r;
    instructions[0xA0] = &CPU::AND_A_r;
    instructions[0xA1] = &CPU::AND_A_r;
    instructions[0xA2] = &CPU::AND_A_r;
    instructions[0xA3] = &CPU::AND_A_r;
    instructions[0xA4] = &CPU::AND_A_r;
    instructions[0xA5] = &CPU::AND_A_r;
    instructions[0xA6] = &CPU::AND_A_HL;
    instructions[0xE6] = &CPU::AND_A_n;

    instructions[0xB7] = &CPU::OR_A_r;
    instructions[0xB0] = &CPU::OR_A_r;
    instructions[0xB1] = &CPU::OR_A_r;
    instructions[0xB2] = &CPU::OR_A_r;
    instructions[0xB3] = &CPU::OR_A_r;
    instructions[0xB4] = &CPU::OR_A_r;
    instructions[0xB5] = &CPU::OR_A_r;
    instructions[0xB6] = &CPU::OR_A_HL;
    instructions[0xF6] = &CPU::OR_A_n;

    instructions[0xAF] = &CPU::XOR_A_r;
    instructions[0xA8] = &CPU::XOR_A_r;
    instructions[0xA9] = &CPU::XOR_A_r;
    instructions[0xAA] = &CPU::XOR_A_r;
    instructions[0xAB] = &CPU::XOR_A_r;
    instructions[0xAC] = &CPU::XOR_A_r;
    instructions[0xAD] = &CPU::XOR_A_r;
    instructions[0xAE] = &CPU::XOR_A_HL;
    instructions[0xEE] = &CPU::XOR_A_n;

    instructions[0xBF] = &CPU::CP_A_r;
    instructions[0xB8] = &CPU::CP_A_r;
    instructions[0xB9] = &CPU::CP_A_r;
    instructions[0xBA] = &CPU::CP_A_r;
    instructions[0xBB] = &CPU::CP_A_r;
    instructions[0xBC] = &CPU::CP_A_r;
    instructions[0xBD] = &CPU::CP_A_r;
    instructions[0xBE] = &CPU::CP_A_HL;
    instructions[0xFE] = &CPU::CP_A_n;

    instructions[0x3C] = &CPU::INC_r;
    instructions[0x04] = &CPU::INC_r;
    instructions[0x0C] = &CPU::INC_r;
    instructions[0x14] = &CPU::INC_r;
    instructions[0x1C] = &CPU::INC_r;
    instructions[0x24] = &CPU::INC_r;
    instructions[0x2C] = &CPU::INC_r;
    instructions[0x34] = &CPU::INC_HL;

    instructions[0x3D] = &CPU::DEC_r;
    instructions[0x05] = &CPU::DEC_r;
    instructions[0x0D] = &CPU::DEC_r;
    instructions[0x15] = &CPU::DEC_r;
    instructions[0x1D] = &CPU::DEC_r;
    instructions[0x25] = &CPU::DEC_r;
    instructions[0x2D] = &CPU::DEC_r;
    instructions[0x35] = &CPU::DEC_HL;

    instructions[0x09] = &CPU::ADD_HL_r2;
    instructions[0x19] = &CPU::ADD_HL_r2;
    instructions[0x29] = &CPU::ADD_HL_r2;
    instructions[0x39] = &CPU::ADD_HL_r2;
    instructions[0xE8] = &CPU::ADD_SP_sn;
    instructions[0x03] = &CPU::INC_r2;
    instructions[0x13] = &CPU::INC_r2;
    instructions[0x23] = &CPU::INC_r2;
    instructions[0x33] = &CPU::INC_r2;
    instructions[0x0B] = &CPU::DEC_r2;
    instructions[0x1B] = &CPU::DEC_r2;
    instructions[0x2B] = &CPU::DEC_r2;
    instructions[0x3B] = &CPU::DEC_r2;

    instructions[0x27] = &CPU::DAA;
    instructions[0x2F] = &CPU::CPL;
    instructions[0x3F] = &CPU::CCF;
    instructions[0x37] = &CPU::SCF;
    instructions[0x00] = &CPU::NOP;
    instructions[0x76] = &CPU::HALT;
    instructions[0x10] = &CPU::STOP;
    instructions[0xF3] = &CPU::DI;
    instructions[0xFB] = &CPU::EI;

    instructions[0x07] = &CPU::RLCA;
    instructions[0x17] = &CPU::RLA;
    instructions[0x0F] = &CPU::RRCA;
    instructions[0x1F] = &CPU::RRA;

    instructions[0xC3] = &CPU::JP_nn;
    instructions[0xC2] = &CPU::JP_cc_nn;
    instructions[0xCA] = &CPU::JP_cc_nn;
    instructions[0xD2] = &CPU::JP_cc_nn;
    instructions[0xDA] = &CPU::JP_cc_nn;
    instructions[0xE9] = &CPU::JP_HL;
    instructions[0x18] = &CPU::JR_sn;
    instructions[0x20] = &CPU::JR_cc_sn;
    instructions[0x28] = &CPU::JR_cc_sn;
    instructions[0x30] = &CPU::JR_cc_sn;
    instructions[0x38] = &CPU::JR_cc_sn;

    instructions[0xCD] = &CPU::CALL_nn;
    instructions[0xC4] = &CPU::CALL_cc_nn;
    instructions[0xCC] = &CPU::CALL_cc_nn;
    instructions[0xD4] = &CPU::CALL_cc_nn;
    instructions[0xDC] = &CPU::CALL_cc_nn;

    instructions[0xC7] = &CPU::RST_n;
    instructions[0xCF] = &CPU::RST_n;
    instructions[0xD7] = &CPU::RST_n;
    instructions[0xDF] = &CPU::RST_n;
    instructions[0xE7] = &CPU::RST_n;
    instructions[0xEF] = &CPU::RST_n;
    instructions[0xF7] = &CPU::RST_n;
    instructions[0xFF] = &CPU::RST_n;
    instructions[0xC9] = &CPU::RET;
    instructions[0xC0] = &CPU::RET_cc;
    instructions[0xC8] = &CPU::RET_cc;
    instructions[0xD0] = &CPU::RET_cc;
    instructions[0xD8] = &CPU::RET_cc;
    instructions[0xD9] = &CPU::RETI;

    // CB Instructions //

    instructionsCB[0x07] = &CPU::RLC_r;     instructionsCB[0x17] = &CPU::RL_r;
    instructionsCB[0x00] = &CPU::RLC_r;     instructionsCB[0x10] = &CPU::RL_r;
    instructionsCB[0x01] = &CPU::RLC_r;     instructionsCB[0x11] = &CPU::RL_r;
    instructionsCB[0x02] = &CPU::RLC_r;     instructionsCB[0x12] = &CPU::RL_r;
    instructionsCB[0x03] = &CPU::RLC_r;     instructionsCB[0x13] = &CPU::RL_r;
    instructionsCB[0x04] = &CPU::RLC_r;     instructionsCB[0x14] = &CPU::RL_r;
    instructionsCB[0x05] = &CPU::RLC_r;     instructionsCB[0x15] = &CPU::RL_r;
    instructionsCB[0x06] = &CPU::RLC_HL;    instructionsCB[0x16] = &CPU::RL_HL;

    instructionsCB[0x0F] = &CPU::RRC_r;     instructionsCB[0x1F] = &CPU::RR_r;
    instructionsCB[0x08] = &CPU::RRC_r;     instructionsCB[0x18] = &CPU::RR_r;
    instructionsCB[0x09] = &CPU::RRC_r;     instructionsCB[0x19] = &CPU::RR_r;
    instructionsCB[0x0A] = &CPU::RRC_r;     instructionsCB[0x1A] = &CPU::RR_r;
    instructionsCB[0x0B] = &CPU::RRC_r;     instructionsCB[0x1B] = &CPU::RR_r;
    instructionsCB[0x0C] = &CPU::RRC_r;     instructionsCB[0x1C] = &CPU::RR_r;
    instructionsCB[0x0D] = &CPU::RRC_r;     instructionsCB[0x1D] = &CPU::RR_r;
    instructionsCB[0x0E] = &CPU::RRC_HL;    instructionsCB[0x1E] = &CPU::RR_HL;

    instructionsCB[0x27] = &CPU::SLA_r;     instructionsCB[0x2F] = &CPU::SRA_r;
    instructionsCB[0x20] = &CPU::SLA_r;     instructionsCB[0x28] = &CPU::SRA_r;
    instructionsCB[0x21] = &CPU::SLA_r;     instructionsCB[0x29] = &CPU::SRA_r;
    instructionsCB[0x22] = &CPU::SLA_r;     instructionsCB[0x2A] = &CPU::SRA_r;
    instructionsCB[0x23] = &CPU::SLA_r;     instructionsCB[0x2B] = &CPU::SRA_r;
    instructionsCB[0x24] = &CPU::SLA_r;     instructionsCB[0x2C] = &CPU::SRA_r;
    instructionsCB[0x25] = &CPU::SLA_r;     instructionsCB[0x2D] = &CPU::SRA_r;
    instructionsCB[0x26] = &CPU::SLA_HL;    instructionsCB[0x2E] = &CPU::SRA_HL;

    instructionsCB[0x3F] = &CPU::SRL_r;     instructionsCB[0x37] = &CPU::SWAP_r;
    instructionsCB[0x38] = &CPU::SRL_r;     instructionsCB[0x30] = &CPU::SWAP_r;
    instructionsCB[0x39] = &CPU::SRL_r;     instructionsCB[0x31] = &CPU::SWAP_r;
    instructionsCB[0x3A] = &CPU::SRL_r;     instructionsCB[0x32] = &CPU::SWAP_r;
    instructionsCB[0x3B] = &CPU::SRL_r;     instructionsCB[0x33] = &CPU::SWAP_r;
    instructionsCB[0x3C] = &CPU::SRL_r;     instructionsCB[0x34] = &CPU::SWAP_r;
    instructionsCB[0x3D] = &CPU::SRL_r;     instructionsCB[0x35] = &CPU::SWAP_r;
    instructionsCB[0x3E] = &CPU::SRL_HL;    instructionsCB[0x36] = &CPU::SWAP_HL;

    instructionsCB[0x40] = &CPU::BIT_b_r;   instructionsCB[0x50] = &CPU::BIT_b_r;
    instructionsCB[0x41] = &CPU::BIT_b_r;   instructionsCB[0x51] = &CPU::BIT_b_r;
    instructionsCB[0x42] = &CPU::BIT_b_r;   instructionsCB[0x52] = &CPU::BIT_b_r;
    instructionsCB[0x43] = &CPU::BIT_b_r;   instructionsCB[0x53] = &CPU::BIT_b_r;
    instructionsCB[0x44] = &CPU::BIT_b_r;   instructionsCB[0x54] = &CPU::BIT_b_r;
    instructionsCB[0x45] = &CPU::BIT_b_r;   instructionsCB[0x55] = &CPU::BIT_b_r;
    instructionsCB[0x46] = &CPU::BIT_b_HL;  instructionsCB[0x56] = &CPU::BIT_b_HL;
    instructionsCB[0x47] = &CPU::BIT_b_r;   instructionsCB[0x57] = &CPU::BIT_b_r;
    instructionsCB[0x48] = &CPU::BIT_b_r;   instructionsCB[0x58] = &CPU::BIT_b_r;
    instructionsCB[0x49] = &CPU::BIT_b_r;   instructionsCB[0x59] = &CPU::BIT_b_r;
    instructionsCB[0x4A] = &CPU::BIT_b_r;   instructionsCB[0x5A] = &CPU::BIT_b_r;
    instructionsCB[0x4B] = &CPU::BIT_b_r;   instructionsCB[0x5B] = &CPU::BIT_b_r;
    instructionsCB[0x4C] = &CPU::BIT_b_r;   instructionsCB[0x5C] = &CPU::BIT_b_r;
    instructionsCB[0x4D] = &CPU::BIT_b_r;   instructionsCB[0x5D] = &CPU::BIT_b_r;
    instructionsCB[0x4E] = &CPU::BIT_b_HL;  instructionsCB[0x5E] = &CPU::BIT_b_HL;
    instructionsCB[0x4F] = &CPU::BIT_b_r;   instructionsCB[0x5F] = &CPU::BIT_b_r;

    instructionsCB[0x60] = &CPU::BIT_b_r;   instructionsCB[0x70] = &CPU::BIT_b_r;
    instructionsCB[0x61] = &CPU::BIT_b_r;   instructionsCB[0x71] = &CPU::BIT_b_r;
    instructionsCB[0x62] = &CPU::BIT_b_r;   instructionsCB[0x72] = &CPU::BIT_b_r;
    instructionsCB[0x63] = &CPU::BIT_b_r;   instructionsCB[0x73] = &CPU::BIT_b_r;
    instructionsCB[0x64] = &CPU::BIT_b_r;   instructionsCB[0x74] = &CPU::BIT_b_r;
    instructionsCB[0x65] = &CPU::BIT_b_r;   instructionsCB[0x75] = &CPU::BIT_b_r;
    instructionsCB[0x66] = &CPU::BIT_b_HL;  instructionsCB[0x76] = &CPU::BIT_b_HL;
    instructionsCB[0x67] = &CPU::BIT_b_r;   instructionsCB[0x77] = &CPU::BIT_b_r;
    instructionsCB[0x68] = &CPU::BIT_b_r;   instructionsCB[0x78] = &CPU::BIT_b_r;
    instructionsCB[0x69] = &CPU::BIT_b_r;   instructionsCB[0x79] = &CPU::BIT_b_r;
    instructionsCB[0x6A] = &CPU::BIT_b_r;   instructionsCB[0x7A] = &CPU::BIT_b_r;
    instructionsCB[0x6B] = &CPU::BIT_b_r;   instructionsCB[0x7B] = &CPU::BIT_b_r;
    instructionsCB[0x6C] = &CPU::BIT_b_r;   instructionsCB[0x7C] = &CPU::BIT_b_r;
    instructionsCB[0x6D] = &CPU::BIT_b_r;   instructionsCB[0x7D] = &CPU::BIT_b_r;
    instructionsCB[0x6E] = &CPU::BIT_b_HL;  instructionsCB[0x7E] = &CPU::BIT_b_HL;
    instructionsCB[0x6F] = &CPU::BIT_b_r;   instructionsCB[0x7F] = &CPU::BIT_b_r;

    instructionsCB[0xC0] = &CPU::SET_b_r;   instructionsCB[0xD0] = &CPU::SET_b_r;
    instructionsCB[0xC1] = &CPU::SET_b_r;   instructionsCB[0xD1] = &CPU::SET_b_r;
    instructionsCB[0xC2] = &CPU::SET_b_r;   instructionsCB[0xD2] = &CPU::SET_b_r;
    instructionsCB[0xC3] = &CPU::SET_b_r;   instructionsCB[0xD3] = &CPU::SET_b_r;
    instructionsCB[0xC4] = &CPU::SET_b_r;   instructionsCB[0xD4] = &CPU::SET_b_r;
    instructionsCB[0xC5] = &CPU::SET_b_r;   instructionsCB[0xD5] = &CPU::SET_b_r;
    instructionsCB[0xC6] = &CPU::SET_b_HL;  instructionsCB[0xD6] = &CPU::SET_b_HL;
    instructionsCB[0xC7] = &CPU::SET_b_r;   instructionsCB[0xD7] = &CPU::SET_b_r;
    instructionsCB[0xC8] = &CPU::SET_b_r;   instructionsCB[0xD8] = &CPU::SET_b_r;
    instructionsCB[0xC9] = &CPU::SET_b_r;   instructionsCB[0xD9] = &CPU::SET_b_r;
    instructionsCB[0xCA] = &CPU::SET_b_r;   instructionsCB[0xDA] = &CPU::SET_b_r;
    instructionsCB[0xCB] = &CPU::SET_b_r;   instructionsCB[0xDB] = &CPU::SET_b_r;
    instructionsCB[0xCC] = &CPU::SET_b_r;   instructionsCB[0xDC] = &CPU::SET_b_r;
    instructionsCB[0xCD] = &CPU::SET_b_r;   instructionsCB[0xDD] = &CPU::SET_b_r;
    instructionsCB[0xCE] = &CPU::SET_b_HL;  instructionsCB[0xDE] = &CPU::SET_b_HL;
    instructionsCB[0xCF] = &CPU::SET_b_r;   instructionsCB[0xDF] = &CPU::SET_b_r;

    instructionsCB[0xE0] = &CPU::SET_b_r;   instructionsCB[0xF0] = &CPU::SET_b_r;
    instructionsCB[0xE1] = &CPU::SET_b_r;   instructionsCB[0xF1] = &CPU::SET_b_r;
    instructionsCB[0xE2] = &CPU::SET_b_r;   instructionsCB[0xF2] = &CPU::SET_b_r;
    instructionsCB[0xE3] = &CPU::SET_b_r;   instructionsCB[0xF3] = &CPU::SET_b_r;
    instructionsCB[0xE4] = &CPU::SET_b_r;   instructionsCB[0xF4] = &CPU::SET_b_r;
    instructionsCB[0xE5] = &CPU::SET_b_r;   instructionsCB[0xF5] = &CPU::SET_b_r;
    instructionsCB[0xE6] = &CPU::SET_b_HL;  instructionsCB[0xF6] = &CPU::SET_b_HL;
    instructionsCB[0xE7] = &CPU::SET_b_r;   instructionsCB[0xF7] = &CPU::SET_b_r;
    instructionsCB[0xE8] = &CPU::SET_b_r;   instructionsCB[0xF8] = &CPU::SET_b_r;
    instructionsCB[0xE9] = &CPU::SET_b_r;   instructionsCB[0xF9] = &CPU::SET_b_r;
    instructionsCB[0xEA] = &CPU::SET_b_r;   instructionsCB[0xFA] = &CPU::SET_b_r;
    instructionsCB[0xEB] = &CPU::SET_b_r;   instructionsCB[0xFB] = &CPU::SET_b_r;
    instructionsCB[0xEC] = &CPU::SET_b_r;   instructionsCB[0xFC] = &CPU::SET_b_r;
    instructionsCB[0xED] = &CPU::SET_b_r;   instructionsCB[0xFD] = &CPU::SET_b_r;
    instructionsCB[0xEE] = &CPU::SET_b_HL;  instructionsCB[0xFE] = &CPU::SET_b_HL;
    instructionsCB[0xEF] = &CPU::SET_b_r;   instructionsCB[0xFF] = &CPU::SET_b_r;

    instructionsCB[0x80] = &CPU::RES_b_r;   instructionsCB[0x90] = &CPU::RES_b_r;
    instructionsCB[0x81] = &CPU::RES_b_r;   instructionsCB[0x91] = &CPU::RES_b_r;
    instructionsCB[0x82] = &CPU::RES_b_r;   instructionsCB[0x92] = &CPU::RES_b_r;
    instructionsCB[0x83] = &CPU::RES_b_r;   instructionsCB[0x93] = &CPU::RES_b_r;
    instructionsCB[0x84] = &CPU::RES_b_r;   instructionsCB[0x94] = &CPU::RES_b_r;
    instructionsCB[0x85] = &CPU::RES_b_r;   instructionsCB[0x95] = &CPU::RES_b_r;
    instructionsCB[0x86] = &CPU::RES_b_HL;  instructionsCB[0x96] = &CPU::RES_b_HL;
    instructionsCB[0x87] = &CPU::RES_b_r;   instructionsCB[0x97] = &CPU::RES_b_r;
    instructionsCB[0x88] = &CPU::RES_b_r;   instructionsCB[0x98] = &CPU::RES_b_r;
    instructionsCB[0x89] = &CPU::RES_b_r;   instructionsCB[0x99] = &CPU::RES_b_r;
    instructionsCB[0x8A] = &CPU::RES_b_r;   instructionsCB[0x9A] = &CPU::RES_b_r;
    instructionsCB[0x8B] = &CPU::RES_b_r;   instructionsCB[0x9B] = &CPU::RES_b_r;
    instructionsCB[0x8C] = &CPU::RES_b_r;   instructionsCB[0x9C] = &CPU::RES_b_r;
    instructionsCB[0x8D] = &CPU::RES_b_r;   instructionsCB[0x9D] = &CPU::RES_b_r;
    instructionsCB[0x8E] = &CPU::RES_b_HL;  instructionsCB[0x9E] = &CPU::RES_b_HL;
    instructionsCB[0x8F] = &CPU::RES_b_r;   instructionsCB[0x9F] = &CPU::RES_b_r;

    instructionsCB[0xA0] = &CPU::RES_b_r;   instructionsCB[0xB0] = &CPU::RES_b_r;
    instructionsCB[0xA1] = &CPU::RES_b_r;   instructionsCB[0xB1] = &CPU::RES_b_r;
    instructionsCB[0xA2] = &CPU::RES_b_r;   instructionsCB[0xB2] = &CPU::RES_b_r;
    instructionsCB[0xA3] = &CPU::RES_b_r;   instructionsCB[0xB3] = &CPU::RES_b_r;
    instructionsCB[0xA4] = &CPU::RES_b_r;   instructionsCB[0xB4] = &CPU::RES_b_r;
    instructionsCB[0xA5] = &CPU::RES_b_r;   instructionsCB[0xB5] = &CPU::RES_b_r;
    instructionsCB[0xA6] = &CPU::RES_b_HL;  instructionsCB[0xB6] = &CPU::RES_b_HL;
    instructionsCB[0xA7] = &CPU::RES_b_r;   instructionsCB[0xB7] = &CPU::RES_b_r;
    instructionsCB[0xA8] = &CPU::RES_b_r;   instructionsCB[0xB8] = &CPU::RES_b_r;
    instructionsCB[0xA9] = &CPU::RES_b_r;   instructionsCB[0xB9] = &CPU::RES_b_r;
    instructionsCB[0xAA] = &CPU::RES_b_r;   instructionsCB[0xBA] = &CPU::RES_b_r;
    instructionsCB[0xAB] = &CPU::RES_b_r;   instructionsCB[0xBB] = &CPU::RES_b_r;
    instructionsCB[0xAC] = &CPU::RES_b_r;   instructionsCB[0xBC] = &CPU::RES_b_r;
    instructionsCB[0xAD] = &CPU::RES_b_r;   instructionsCB[0xBD] = &CPU::RES_b_r;
    instructionsCB[0xAE] = &CPU::RES_b_HL;  instructionsCB[0xBE] = &CPU::RES_b_HL;
    instructionsCB[0xAF] = &CPU::RES_b_r;   instructionsCB[0xBF] = &CPU::RES_b_r;

}

bool CPU::init(std::string& romPath) {
    // TODO: remove hardcoded BIOS path
    std::string biosPath = "../../gb/BootROM.gb";

    // MMU reset
    if (!mmu.init(romPath, biosPath)) return false;
    // CPU reset
    reset();
    // GPU reset
    gpu.reset();
    // Joypad reset
    joypad.reset();
    // APU reset
    apu.reset();

    return true;
}

void CPU::reset() {
    // startup values (https://problemkaputt.de/pandocs.htm#powerupsequence)
    r.af = (gbMode == DMG) ? 0x01B0 : 0x11B0;
    r.bc = 0x0013;
    r.de = 0x00D8;
    r.hl = 0x014D;
    r.sp = 0xFFFE;
    r.ime = 0x0;
    r.pc = mmu.inBIOS ? 0x0000 : 0x0100;

    cycles = 0;
    ticksPerFrame = 70224;
    timerCounter = 1024;
    dividerCounter = 0;

    runCGBinDMGMode = false;
    doubleSpeedMode = false;

    writeByte(0xFF05, 0x00);    // TIMA
    writeByte(0xFF06, 0x00);    // TMA
    writeByte(0xFF07, 0x00);    // TAC

    writeByte(0xFF10, 0x80);    // NR10
    writeByte(0xFF11, 0xBF);    // NR11
    writeByte(0xFF12, 0xF3);    // NR12
    writeByte(0xFF14, 0xBF);    // NR14
    writeByte(0xFF16, 0x3F);    // NR21
    writeByte(0xFF17, 0x00);    // NR22
    writeByte(0xFF19, 0xBF);    // NR24
    writeByte(0xFF1A, 0x7F);    // NR30
    writeByte(0xFF1B, 0xFF);    // NR31
    writeByte(0xFF1C, 0x9F);    // NR32
    writeByte(0xFF1E, 0xBF);    // NR33
    writeByte(0xFF20, 0xFF);    // NR41
    writeByte(0xFF21, 0x00);    // NR42
    writeByte(0xFF22, 0x00);    // NR43
    writeByte(0xFF23, 0xBF);    // NR30
    writeByte(0xFF24, 0x77);    // NR50
    writeByte(0xFF25, 0xF3);    // NR51
    writeByte(0xFF26, 0xF1);    // NR52 (0xF0 for SGB)

    writeByte(0xFF40, 0x91);    // LCDC
    writeByte(0xFF42, 0x00);    // SCY
    writeByte(0xFF43, 0x00);    // SCY
    writeByte(0xFF45, 0x00);    // LYC
    writeByte(0xFF47, 0xFC);    // BGP
    writeByte(0xFF48, 0xFF);    // OBP0
    writeByte(0xFF49, 0xFF);    // OBP1
    writeByte(0xFF4A, 0x00);    // WY
    writeByte(0xFF4B, 0x00);    // WX

    if (gbMode == DMG) {
        writeByte(0xFF6C, 0xFF);
        writeByte(0xFF74, 0xFF);
    }
    if (gbMode == CGB) {
        writeByte(0xFF6C, 0xFE);
    }

    writeByte(0xFFFF, 0x00);    // IE
}

void CPU::handleInputDown(u8 key) {
    joypad.handleInputDown(key);
}

void CPU::handleInputUp(u8 key) {
    joypad.handleInputUp(key);
}

u32 CPU::tick() {
    cycles = 0;
    u8 opcode = 0;
    bool isCBInstruction = false;
    if (halted) {
        cycles = NOP(0x00);
    } else {
        opcode = readByte(r.pc++);
        Instruction instruction;
        if (opcode == 0xCB) {
            isCBInstruction = true;
            opcode = readByte(r.pc++);
            instruction = instructionsCB[opcode];
        } else {
            instruction = instructions[opcode];
        }

        if (instruction == nullptr) {
            Log(F, "Invalid opcode 0x%02X at address 0x%04X\n", opcode, r.pc-1);
            return 0;
        }
        cycles = (this->*instruction)(opcode);
    }

    if (cycles == 0) {
        Log(F, "Unimplemented opcode");
        if (isCBInstruction) LogRaw(F, " 0xCB");
        LogRaw(F, " 0x%02X at address 0x%02X\n", opcode, r.pc-1);
        return 0;
    }

    gpu.tick(cycles);

    updateTimer(cycles);

    apu.update(cycles);

    checkInterrupts();

    return cycles;
}

void CPU::updateTimer(u32 ticks) {
    dividerCounter += ticks;
    if (dividerCounter >= 0xFF) {
        dividerCounter -= 0xFF;
        // access divider directly
        mmu.IO[0x4]++;
    }

    u8 tac = readByte(0xFF07);
    if (!isBitSet(tac, 2)) return;
    static bool delayedOverflow = false;

    for (u32 i=0; i<ticks; i++) {
        timerCounter--;

        if (delayedOverflow) {
            writeByte(0xFF05, readByte(0xFF06));
            requestInterrupt(INTERRUPT_TIMER);
            delayedOverflow = false;
        }

        if (timerCounter <= 0) {
            int remainder = timerCounter;
            setTimerFreq();
            timerCounter += remainder;

            writeByte(0xFF05, readByte(0xFF05) + 1);
            delayedOverflow = (readByte(0xFF05) == 0);
        }
    }
}

void CPU::setTimerFreq() {
    u8 freq = readByte(0xFF07) & 0x03;
    switch (freq) {
        case 0: timerCounter = 1024; break;
        case 1: timerCounter = 16;   break;
        case 2: timerCounter = 64;   break;
        case 3: timerCounter = 256;  break;
        default: Log(W, "Invalid timer frequency\n");
    }
}

void CPU::checkInterrupts() {
    u8 IE = mmu.readByte(0xFFFF);
    u8 IF = mmu.readByte(0xFF0F);
    // filter active interrupts
    u8 activeInterrupts = (IE & IF) & 0x1F;
    if (r.ime) {
        if (activeInterrupts > 0) {
            if (halted) cycles += 4;
            cycles += 20;
            halted = false;
            r.ime = 0;
            pushWord(r.pc);

            if (isBitSet(activeInterrupts, 0)) {
                r.pc = INTERRUPT_VBLANK;
                IF = clearBit(IF, 0);
            } else if (isBitSet(activeInterrupts, 1)) {
                r.pc = INTERRUPT_LCD_STAT;
                IF = clearBit(IF, 1);
            } else if (isBitSet(activeInterrupts, 2)) {
                r.pc = INTERRUPT_TIMER;
                IF = clearBit(IF, 2);
            } else if (isBitSet(activeInterrupts, 3)) {
                r.pc = INTERRUPT_SERIAL;
                IF = clearBit(IF, 3);
            } else if (isBitSet(activeInterrupts, 4)) {
                r.pc = INTERRUPT_JOYPAD;
                IF = clearBit(IF, 4);
            }
            mmu.writeByte(0xFF0F, IF);
        }
    } else {
        if (activeInterrupts) halted = false;
        // TODO: HALT bug
    }
}

void CPU::requestInterrupt(u8 interrupt) {
    u8 IF = mmu.readByte(0xFF0F);
    if (interrupt == INTERRUPT_VBLANK) IF = setBit(IF, 0);
    else if (interrupt == INTERRUPT_LCD_STAT) IF = setBit(IF, 1);
    else if (interrupt == INTERRUPT_TIMER) IF = setBit(IF, 2);
    else if (interrupt == INTERRUPT_SERIAL) IF = setBit(IF, 3);
    else if (interrupt == INTERRUPT_JOYPAD) IF = setBit(IF, 4);

    if (r.ime == 0 && ((mmu.readByte(0xFFFF) & IF) & 0x1F) == 0) halted = false;

    mmu.writeByte(0xFF0F, IF);
}

void CPU::pushByte(u8 value) {
    r.sp--;
    writeByte(r.sp, value);
}

void CPU::pushWord(u16 value) {
    pushByte((value >> 8) & 0xFF);
    pushByte(value & 0xFF);
}

u8 CPU::popByte() {
    u8 value = readByte(r.sp);
    r.sp++;
    return value;
}

u16 CPU::popWord() {
    u16 value = readWord(r.sp);
    r.sp += 2;
    return value;
}

void CPU::setFlag(FLAG flag) {
    r.f |= flag;
}

void CPU::clearFlag(FLAG flag) {
    r.f &= ~flag;
}

bool CPU::isFlagSet(FLAG flag) {
    return r.f & flag;
}

u8 CPU::readByte(u16 address) {
    if (address == JOYPAD_ADDRESS) {
        return joypad.readByte();
    } else {
        return mmu.readByte(address);
    }
}

void CPU::writeByte(u16 address, u8 value) {
    if (address == JOYPAD_ADDRESS) {
        joypad.writeByte(value);
    } else if (address == 0xFF07) {
        u8 currentFreq = mmu.readByte(0xFF07) & (u8) 0x03;
        mmu.writeByte(address, value);
        u8 newFreq = mmu.readByte(0xFF07) & (u8) 0x03;
        if (currentFreq != newFreq) setTimerFreq();
    } else if (address >= 0xFF10 && address <= 0xFF26) {
        u8 type = address & 0xFF;
        switch (type) {
            case CH1_FREQ_HIGH:
            case CH2_FREQ_HIGH:
            case CH3_FREQ_HIGH:
            case CH4_COUNTER_INITIAL:
                if (isBitSet(value, 7)) apu.channels[(type - 0x14) / 5]->reset();
                break;
            case CH3_SOUND_ON_OFF:
                if (!isBitSet(value, 7)) apu.channels[2]->on = false;
                break;
            case CH3_SOUND_LENGTH:
                apu.channels[2]->lengthCounter = 0xFF - value;
                break;
            case CH3_OUTPUT_LVL_SELECT:
                apu.channels[2]->volume = Channel::volumeShifts[(value >> 5) & 0x3];
                break;
            case CH1_SOUND_LENGTH:
            case CH2_SOUND_LENGTH:
            case CH4_SOUND_LENGTH:
                apu.channels[(type - 0x11) / 5]->lengthCounter = 0x40 - (value & 0x3F);
                break;
            case OUTPUT_SELECT:
                for (u8 i = 0; i < 4; ++i) {
                    apu.channels[i]->onLeft = isBitSet(value, 4 + i);
                    apu.channels[i]->onRight = isBitSet(value, i);
                }
                break;
            default:
                break;
        }
        // TODO: R/W
        mmu.writeByte(address, value);
    } else {
        mmu.writeByte(address, value);
    }
}

u16 CPU::readWord(u16 address) {
    return mmu.readWord(address);
}

void CPU::writeWord(u16 address, u16 value) {
    mmu.writeWord(address, value);
}

u8* CPU::byteRegister(u8 opcode) {
    return byteRegisterMap[opcode & 0x7];
}

u16* CPU::wordRegister(u8 opcode) {
    return shortRegisterMap[opcode & 0x3];
}

// TODO: remove these two functions
void CPU::checkHalfCarry(u8 reg) {
    ((r.a & 0x0F) < (reg & 0x0F)) ? setFlag(HALF_CARRY) : clearFlag(HALF_CARRY);
}

void CPU::checkCarry(u8 reg) {
    ((r.a & 0xFF) < (reg & 0xFF)) ? setFlag(CARRY) : clearFlag(CARRY);
}

void CPU::serialize(serializer &s) {
    s.integer(r.af);
    s.integer(r.bc);
    s.integer(r.de);
    s.integer(r.hl);
    s.integer(r.pc);
    s.integer(r.sp);
    s.enumeration(gbMode);
    s.integer(cycles);
    s.integer(ticksPerFrame);
    s.integer(halted);
    s.integer(timerCounter);
    s.integer(dividerCounter);
    s.integer(headless);
    s.integer(runCGBinDMGMode);
    s.integer(doubleSpeedMode);
}

// CPU Instructions //

u32 CPU::LD_r_n(const u8& opcode) {
    u8 n = readByte(r.pc++);
    u8* reg = byteRegister(opcode >> 3);
    *reg = n;
    return 8;
}

u32 CPU::LD_r_r(const u8& opcode) {
    u8* dest = byteRegister(opcode >> 3);
    u8* src = byteRegister(opcode);
    *dest = *src;
    return 4;
}

u32 CPU::LD_r_HL(const u8& opcode) {
    u8* reg = byteRegister(opcode >> 3);
    *reg = readByte(r.hl);
    return 8;
}

u32 CPU::LD_HL_r(const u8& opcode) {
    u8* reg = byteRegister(opcode);
    writeByte(r.hl, *reg);
    return 8;
}

u32 CPU::LD_HL_n(const u8& opcode) {
    u8 n = readByte(r.pc++);
    writeByte(r.hl, n);
    return 12;
}

u32 CPU::LD_A_BC(const u8& opcode) {
    r.a = readByte(r.bc);
    return 8;
}

u32 CPU::LD_A_DE(const u8& opcode) {
    r.a = readByte(r.de);
    return 8;
}

u32 CPU::LD_A_nn(const u8& opcode) {
    u16 address = readWord(r.pc);
    r.pc += 2;
    r.a = readByte(address);
    return 16;
}

u32 CPU::LD_BC_A(const u8& opcode) {
    writeByte(r.bc, r.a);
    return 8;
}

u32 CPU::LD_DE_A(const u8& opcode) {
    writeByte(r.de, r.a);
    return 8;
}

u32 CPU::LD_nn_A(const u8& opcode) {
    u16 address = readWord(r.pc);
    r.pc += 2;
    writeByte(address, r.a);
    return 16;
}

u32 CPU::LD_A_Cff00(const u8& opcode) {
    r.a = readByte(0xFF00 + r.c);
    return 8;
}

u32 CPU::LD_Cff00_A(const u8& opcode) {
    writeByte(0xFF00 + r.c, r.a);
    return 8;
}

u32 CPU::LDD_A_HL(const u8& opcode) {
    r.a = readByte(r.hl);
    r.hl--;
    return 8;
}

u32 CPU::LDD_HL_A(const u8& opcode) {
    writeByte(r.hl, r.a);
    r.hl--;
    return 8;
}

u32 CPU::LDI_A_HL(const u8& opcode) {
    r.a = readByte(r.hl);
    r.hl++;
    return 8;
}

u32 CPU::LDI_HL_A(const u8& opcode) {
    writeByte(r.hl, r.a);
    r.hl++;
    return 8;
}

u32 CPU::LD_nff00_A(const u8& opcode) {
    u16 address = (u16) 0xFF00 + readByte(r.pc++);
    writeByte(address, r.a);
    return 12;
}

u32 CPU::LD_A_nff00(const u8& opcode) {
    u16 address = (u16) 0xFF00 + readByte(r.pc++);
    r.a = readByte(address);
    return 12;
}

u32 CPU::LD_r2_nn(const u8& opcode) {
    u16 nn = readWord(r.pc);
    r.pc += 2;
    u16* reg = wordRegister(opcode >> 4);
    *reg = nn;
    return 12;
}

u32 CPU::LD_SP_HL(const u8& opcode) {
    r.sp = r.hl;
    return 8;
}

u32 CPU::LD_HL_SPn(const u8& opcode) {
    u8 n = readByte(r.pc++);
    char sn = static_cast<char>(n);
    u16 result = r.sp + sn;

    clearFlag(ZERO);
    clearFlag(ADD_SUB);

    (((r.sp ^ sn ^ result) & 0x10) != 0) ? setFlag(HALF_CARRY) : clearFlag(HALF_CARRY);
    (((r.sp ^ sn ^ result) & 0x100) != 0) ? setFlag(CARRY) : clearFlag(CARRY);

    r.hl = result;
    return 12;
}

u32 CPU::LD_nn_SP(const u8& opcode) {
    u16 address = readWord(r.pc);
    r.pc += 2;
    writeWord(address, r.sp);
    return 20;
}

u32 CPU::PUSH_r2(const u8& opcode) {
    u8 opcodeIndex = (opcode >> 4) & 0x3;
    u16 reg;
    if (opcodeIndex == 0x3) reg = r.af;
    else reg = *(wordRegister(opcodeIndex));

    pushWord(reg);
    return 16;
}

u32 CPU::POP_r2(const u8& opcode) {
    u8 opcodeIndex = (opcode >> 4) & 0x3;
    u16* reg;
    if (opcodeIndex == 0x3) reg = &r.af;
    else reg = wordRegister(opcodeIndex);

    (*reg) = popWord();
    if (opcodeIndex == 0x3) *reg &= 0xFFF0;

    return 12;
}

u32 CPU::ADD_A_r(const u8& opcode) {
    u8* reg = byteRegister(opcode);
    u8 result = r.a + *reg;

    (result == 0) ? setFlag(ZERO) : clearFlag(ZERO);
    clearFlag(ADD_SUB);
    (((result ^ (*reg) ^ r.a) & 0x10) == 0x10) ? setFlag(HALF_CARRY) : clearFlag(HALF_CARRY);
    (result < r.a) ? setFlag(CARRY) : clearFlag(CARRY);

    r.a = result;
    return 4;
}

u32 CPU::ADD_A_HL(const u8& opcode) {
    u8 value = readByte(r.hl);
    u8 result = r.a + value;

    (result == 0) ? setFlag(ZERO) : clearFlag(ZERO);
    clearFlag(ADD_SUB);
    (((result ^ value ^ r.a) & 0x10) == 0x10) ? setFlag(HALF_CARRY) : clearFlag(HALF_CARRY);
    (result < r.a) ? setFlag(CARRY) : clearFlag(CARRY);

    r.a = result;
    return 8;
}

u32 CPU::ADD_A_n(const u8& opcode) {
    u8 value = readByte(r.pc++);
    u8 result = r.a + value;

    (result == 0) ? setFlag(ZERO) : clearFlag(ZERO);
    clearFlag(ADD_SUB);
    (((result ^ value ^ r.a) & 0x10) == 0x10) ? setFlag(HALF_CARRY) : clearFlag(HALF_CARRY);
    (result < r.a) ? setFlag(CARRY) : clearFlag(CARRY);

    r.a = result;
    return 8;
}

u32 CPU::ADC_A_r(const u8& opcode) {
    u8* reg = byteRegister(opcode);
    u8 carry = isFlagSet(CARRY) ? 1 : 0;

    if (((int)(r.a & 0x0F) + (int)(*reg & 0x0F) + (int)carry) > 0x0F) setFlag(HALF_CARRY);
    else clearFlag(HALF_CARRY);

    if (((int)(r.a & 0xFF) + (int)(*reg & 0xFF) + (int)carry) > 0xFF) setFlag(CARRY);
    else clearFlag(CARRY);

    r.a = r.a + *reg + carry;

    (r.a == 0x00) ? setFlag(ZERO) : clearFlag(ZERO);
    clearFlag(ADD_SUB);

    return 4;
}

u32 CPU::ADC_A_HL(const u8& opcode) {
    u8 value = readByte(r.hl);
    u8 carry = isFlagSet(CARRY) ? 1 : 0;

    if (((int)(r.a & 0x0F) + (int)(value & 0x0F) + (int)carry) > 0x0F) setFlag(HALF_CARRY);
    else clearFlag(HALF_CARRY);

    if (((int)(r.a & 0xFF) + (int)(value & 0xFF) + (int)carry) > 0xFF) setFlag(CARRY);
    else clearFlag(CARRY);

    r.a = r.a + value + carry;

    (r.a == 0x00) ? setFlag(ZERO) : clearFlag(ZERO);
    clearFlag(ADD_SUB);

    return 8;
}

u32 CPU::ADC_A_n(const u8& opcode) {
    u8 n = readByte(r.pc++);
    u8 carry = isFlagSet(CARRY) ? 1 : 0;

    if (((int)(r.a & 0x0F) + (int)(n & 0x0F) + (int)carry) > 0x0F) setFlag(HALF_CARRY);
    else clearFlag(HALF_CARRY);

    if (((int)(r.a & 0xFF) + (int)(n & 0xFF) + (int)carry) > 0xFF) setFlag(CARRY);
    else clearFlag(CARRY);

    r.a = r.a + n + carry;

    (r.a == 0x00) ? setFlag(ZERO) : clearFlag(ZERO);
    clearFlag(ADD_SUB);

    return 8;
}

u32 CPU::SUB_A_r(const u8& opcode) {
    u8* reg = byteRegister(opcode);
    u8 result  = r.a - *reg;
    (result == 0) ? setFlag(ZERO) : clearFlag(ZERO);
    setFlag(ADD_SUB);
    checkHalfCarry(*reg);
    checkCarry(*reg);
    r.a = result;
    return 4;
}

u32 CPU::SUB_A_HL(const u8& opcode) {
    u8 value = readByte(r.hl);
    u8 result = r.a - value;
    (result == 0) ? setFlag(ZERO) : clearFlag(ZERO);
    setFlag(ADD_SUB);
    checkHalfCarry(value);
    checkCarry(value);
    r.a = result;
    return 8;
}

u32 CPU::SUB_A_n(const u8& opcode) {
    u8 value = readByte(r.pc++);
    u8 result = r.a - value;

    (result == 0) ? setFlag(ZERO) : clearFlag(ZERO);
    setFlag(ADD_SUB);
    checkHalfCarry(value);
    checkCarry(value);

    r.a = result;

    return 8;
}

u32 CPU::SBC_A_r(const u8& opcode) {
    u8* reg = byteRegister(opcode);
    int un = *reg & 0xFF;
    int tmpa = r.a & 0xFF;
    int ua = tmpa;

    ua -= un;

    if (isFlagSet(CARRY)) ua -= 1;

    (ua < 0) ? setFlag(CARRY) : clearFlag(CARRY);

    ua &= 0xFF;

    (ua == 0x00) ? setFlag(ZERO) : clearFlag(ZERO);

    if (((ua ^ un ^ tmpa) & 0x10) == 0x10) setFlag(HALF_CARRY);
    else clearFlag(HALF_CARRY);

    r.a = (u8) ua;
    setFlag(ADD_SUB);

    return 4;
}

u32 CPU::SBC_A_HL(const u8& opcode) {
    int un = readByte(r.hl) & 0xFF;
    int tmpa = r.a & 0xFF;
    int ua = tmpa;

    ua -= un;

    if (isFlagSet(CARRY)) ua -= 1;

    (ua < 0) ? setFlag(CARRY) : clearFlag(CARRY);

    ua &= 0xFF;

    (ua == 0x00) ? setFlag(ZERO) : clearFlag(ZERO);

    if (((ua ^ un ^ tmpa) & 0x10) == 0x10) setFlag(HALF_CARRY);
    else clearFlag(HALF_CARRY);

    r.a = (u8) ua;
    setFlag(ADD_SUB);

    return 8;
}

u32 CPU::SBC_A_n(const u8& opcode) {
    int un = readByte(r.pc++) & 0xFF;
    int tmpa = r.a & 0xFF;
    int ua = tmpa;

    ua -= un;

    if (isFlagSet(CARRY)) ua -= 1;

    (ua < 0) ? setFlag(CARRY) : clearFlag(CARRY);

    ua &= 0xFF;

    (ua == 0x00) ? setFlag(ZERO) : clearFlag(ZERO);

    if (((ua ^ un ^ tmpa) & 0x10) == 0x10) setFlag(HALF_CARRY);
    else clearFlag(HALF_CARRY);

    r.a = (u8) ua;
    setFlag(ADD_SUB);

    return 8;
}

u32 CPU::AND_A_r(const u8& opcode) {
    u8* reg = byteRegister(opcode);
    r.a &= *reg;
    if (r.a == 0x0) setFlag(ZERO);
    else clearFlag(ZERO);
    clearFlag(ADD_SUB);
    setFlag(HALF_CARRY);
    clearFlag(CARRY);

    return 4;
}

u32 CPU::AND_A_HL(const u8& opcode) {
    u8 value = readByte(r.hl);
    r.a &= value;
    if (r.a == 0x0) setFlag(ZERO);
    else clearFlag(ZERO);
    clearFlag(ADD_SUB);
    setFlag(HALF_CARRY);
    clearFlag(CARRY);

    return 8;
}

u32 CPU::AND_A_n(const u8& opcode) {
    u8 n = readByte(r.pc++);
    r.a &= n;
    (r.a == 0x0) ? setFlag(ZERO) : clearFlag(ZERO);
    clearFlag(ADD_SUB);
    setFlag(HALF_CARRY);
    clearFlag(CARRY);
    return 8;
}

u32 CPU::OR_A_r(const u8& opcode) {
    u8* reg = byteRegister(opcode);
    r.a |= *reg;
    if (r.a == 0x0) setFlag(ZERO);
    else clearFlag(ZERO);
    clearFlag(ADD_SUB);
    clearFlag(HALF_CARRY);
    clearFlag(CARRY);

    return 4;
}

u32 CPU::OR_A_HL(const u8& opcode) {
    u8 value = readByte(r.hl);
    r.a |= value;
    if (r.a == 0x0) setFlag(ZERO);
    else clearFlag(ZERO);
    clearFlag(ADD_SUB);
    clearFlag(HALF_CARRY);
    clearFlag(CARRY);

    return 8;
}

u32 CPU::OR_A_n(const u8& opcode) {
    u8 n = readByte(r.pc++);
    r.a |= n;
    if (r.a == 0x0) setFlag(ZERO);
    else clearFlag(ZERO);
    clearFlag(ADD_SUB);
    clearFlag(HALF_CARRY);
    clearFlag(CARRY);

    return 8;
}

u32 CPU::XOR_A_r(const u8& opcode) {
    u8* reg = byteRegister(opcode);
    r.a ^= *reg;
    if (r.a == 0x0) setFlag(ZERO);
    else clearFlag(ZERO);
    clearFlag(ADD_SUB);
    clearFlag(HALF_CARRY);
    clearFlag(CARRY);

    return 4;
}

u32 CPU::XOR_A_HL(const u8& opcode) {
    u8 value = readByte(r.hl);
    r.a ^= value;
    if (r.a == 0x0) setFlag(ZERO);
    else clearFlag(ZERO);
    clearFlag(ADD_SUB);
    clearFlag(HALF_CARRY);
    clearFlag(CARRY);

    return 8;
}

u32 CPU::XOR_A_n(const u8& opcode) {
    u8 n = readByte(r.pc++);
    r.a ^= n;
    if (r.a == 0x0) setFlag(ZERO);
    else clearFlag(ZERO);
    clearFlag(ADD_SUB);
    clearFlag(HALF_CARRY);
    clearFlag(CARRY);

    return 8;
}

u32 CPU::CP_A_r(const u8& opcode) {
    u8* reg = byteRegister(opcode);
    u8 result = r.a - *reg;
    if (result == 0x0) setFlag(ZERO);
    else clearFlag(ZERO);
    setFlag(ADD_SUB);
    if (r.a < *reg) setFlag(CARRY);
    else clearFlag(CARRY);
    if ((r.a & 0xF) < (*reg & 0xF)) setFlag(HALF_CARRY);
    else clearFlag(HALF_CARRY);

    return 4;
}

u32 CPU::CP_A_HL(const u8& opcode) {
    u8 value = readByte(r.hl);
    u8 result = r.a - value;
    if (result == 0x0) setFlag(ZERO);
    else clearFlag(ZERO);
    setFlag(ADD_SUB);
    if (r.a < value) setFlag(CARRY);
    else clearFlag(CARRY);
    if ((r.a & 0xF) < (value & 0xF)) setFlag(HALF_CARRY);
    else clearFlag(HALF_CARRY);

    return 8;
}

u32 CPU::CP_A_n(const u8& opcode) {
    u8 n = readByte(r.pc++);
    u8 result = r.a - n;
    if (result == 0x0) setFlag(ZERO);
    else clearFlag(ZERO);
    setFlag(ADD_SUB);
    if (r.a < n) setFlag(CARRY);
    else clearFlag(CARRY);
    if ((r.a & 0xF) < (n & 0xF)) setFlag(HALF_CARRY);
    else clearFlag(HALF_CARRY);

    return 8;
}

u32 CPU::INC_r(const u8& opcode) {
    u8* reg = byteRegister(opcode >> 3);
    u8 result = *reg + 1;

    (result == 0) ? setFlag(ZERO) : clearFlag(ZERO);
    clearFlag(ADD_SUB);
    if ((*reg & 0xF) + 0x1 > 0xF) setFlag(HALF_CARRY);
    else clearFlag(HALF_CARRY);
    *reg = result;

    return 4;
}

u32 CPU::INC_HL(const u8& opcode) {
    u8 result = readByte(r.hl) + 1;

    (result == 0) ? setFlag(ZERO) : clearFlag(ZERO);
    clearFlag(ADD_SUB);
    if ((readByte(r.hl) & 0xF) + 0x1 > 0xF) setFlag(HALF_CARRY);
    else clearFlag(HALF_CARRY);

    writeByte(r.hl, result);
    return 12;
}

u32 CPU::DEC_r(const u8& opcode) {
    u8* reg = byteRegister(opcode >> 3);
    u8 result = *reg - 1;
    (result == 0) ? setFlag(ZERO) : clearFlag(ZERO);
    setFlag(ADD_SUB);
    if ((*reg & 0xF) - 0x1 < 0) setFlag(HALF_CARRY);
    else clearFlag(HALF_CARRY);
    *reg = result;

    return 4;
}

u32 CPU::DEC_HL(const u8& opcode) {
    u8 result = readByte(r.hl) - 1;

    (result == 0) ? setFlag(ZERO) : clearFlag(ZERO);
    setFlag(ADD_SUB);
    if ((readByte(r.hl) & 0xF) - 0x1 < 0) setFlag(HALF_CARRY);
    else clearFlag(HALF_CARRY);

    writeByte(r.hl, result);
    return 12;
}

u32 CPU::ADD_HL_r2(const u8& opcode) {
    u16* reg = wordRegister(opcode >> 4);
    u16 result = r.hl + *reg;

    clearFlag(ADD_SUB);
    (result < r.hl) ? setFlag(CARRY) : clearFlag(CARRY);
    ((result ^ r.hl ^ (*reg)) & 0x1000) ? setFlag(HALF_CARRY) : clearFlag(HALF_CARRY);

    r.hl = result;
    return 8;
}

u32 CPU::ADD_SP_sn(const u8& opcode) {
    char sn = static_cast<char>(readByte(r.pc++));
    u16 result = r.sp + sn;

    clearFlag(ZERO);
    clearFlag(ADD_SUB);
    ((result & 0xF) < (r.sp & 0xF)) ? setFlag(HALF_CARRY) : clearFlag(HALF_CARRY);
    ((result & 0xFF) < (r.sp & 0xFF)) ? setFlag(CARRY) : clearFlag(CARRY);

    r.sp = result;
    return 16;
}

u32 CPU::INC_r2(const u8& opcode) {
    u16* reg = wordRegister(opcode >> 4);
    (*reg)++;
    return 8;
}

u32 CPU::DEC_r2(const u8& opcode) {
    u16* reg = wordRegister(opcode >> 4);
    (*reg)--;
    return 8;
}

u32 CPU::DAA(const u8& opcode) {
    if (!isFlagSet(ADD_SUB)) {
        if (isFlagSet(CARRY) || r.a > 0x99) {
            r.a += 0x60;
            setFlag(CARRY);
        }
        if (isFlagSet(HALF_CARRY) || (r.a & 0x0F) > 0x09) r.a += 0x06;
    } else {
        if (isFlagSet(CARRY)) r.a -= 0x60;
        if (isFlagSet(HALF_CARRY)) r.a -= 0x06;
    }

    (r.a == 0) ? setFlag(ZERO) : clearFlag(ZERO);
    clearFlag(HALF_CARRY);

    return 4;
}

u32 CPU::CPL(const u8& opcode) {
    r.a ^= 0xFF;
    setFlag(ADD_SUB);
    setFlag(HALF_CARRY);
    return 4;
}

u32 CPU::CCF(const u8& opcode) {
    isFlagSet(CARRY) ? clearFlag(CARRY) : setFlag(CARRY);
    clearFlag(HALF_CARRY);
    clearFlag(ADD_SUB);
    return 4;
}

u32 CPU::SCF(const u8& opcode) {
    setFlag(CARRY);
    clearFlag(HALF_CARRY);
    clearFlag(ADD_SUB);
    return 4;
}

u32 CPU::NOP(const u8& opcode) {
    return 4;
}

u32 CPU::HALT(const u8& opcode) {
    halted = true;
    return 4;
}

u32 CPU::STOP(const u8& opcode) {
    halted = true;
    if (gbMode == CGB && isBitSet(mmu.IO[0x4D], 0)) {
        doubleSpeedMode = !isBitSet(mmu.IO[0x4D], 7);
        mmu.IO[0x4D]--;
        mmu.IO[0x4D] = doubleSpeedMode ? setBit(mmu.IO[0x4D], 7) : clearBit(mmu.IO[0x4D], 7);
        ticksPerFrame = doubleSpeedMode ? (70224 * 2) : 70224;
        apu.reset();
    }
    return 4;
}

u32 CPU::DI(const u8& opcode) {
    r.ime = 0;
    return 4;
}

u32 CPU::EI(const u8& opcode) {
    r.ime = 1;
    return 4;
}

u32 CPU::RLCA(const u8& opcode) {
    isBitSet(r.a, 7) ? setFlag(CARRY) : clearFlag(CARRY);

    r.a <<= 1;
    r.a = isFlagSet(CARRY) ? setBit(r.a, 0) : clearBit(r.a, 0);

    clearFlag(ADD_SUB);
    clearFlag(HALF_CARRY);
    clearFlag(ZERO);

    return 4;
}

u32 CPU::RLA(const u8& opcode) {
    bool oldCarry = isFlagSet(CARRY);
    isBitSet(r.a, 7) ? setFlag(CARRY) : clearFlag(CARRY);

    r.a <<= 1;
    r.a = oldCarry ? setBit(r.a, 0) : clearBit(r.a, 0);

    clearFlag(ADD_SUB);
    clearFlag(HALF_CARRY);
    clearFlag(ZERO);

    return 4;
}

u32 CPU::RRCA(const u8& opcode) {
    isBitSet(r.a, 0) ? setFlag(CARRY) : clearFlag(CARRY);

    r.a >>= 1;
    r.a = isFlagSet(CARRY) ? setBit(r.a, 7) : clearBit(r.a, 7);

    clearFlag(ADD_SUB);
    clearFlag(HALF_CARRY);
    clearFlag(ZERO);

    return 4;
}

u32 CPU::RRA(const u8& opcode) {
    bool oldCarry = isFlagSet(CARRY);
    isBitSet(r.a, 0) ? setFlag(CARRY) : clearFlag(CARRY);

    r.a >>= 1;
    r.a = oldCarry ? setBit(r.a, 7) : clearBit(r.a, 7);

    clearFlag(ADD_SUB);
    clearFlag(HALF_CARRY);
    clearFlag(ZERO);

    return 4;
}

u32 CPU::JP_nn(const u8& opcode) {
    r.pc = readWord(r.pc);
    return 16;
}

u32 CPU::JP_cc_nn(const u8& opcode) {
    u16 nn = readWord(r.pc);
    r.pc += 2;

    bool jump = false;
    u8 condition = (opcode >> 3) & 0x3;
    if (condition == 0x0) jump = !isFlagSet(ZERO);
    else if (condition == 0x1) jump = isFlagSet(ZERO);
    else if (condition == 0x2) jump = !isFlagSet(CARRY);
    else if (condition == 0x3) jump = isFlagSet(CARRY);

    if (jump) {
        r.pc = nn;
        return 16;
    } else {
        return 12;
    }
}

u32 CPU::JP_HL(const u8& opcode) {
    r.pc = r.hl;
    return 4;
}

u32 CPU::JR_sn(const u8& opcode) {
    char sn = static_cast<char>(readByte(r.pc++));
    r.pc += sn;

    return 12;
}

u32 CPU::JR_cc_sn(const u8& opcode) {
    char sn = static_cast<char>(readByte(r.pc++));

    bool jump = false;
    u8 condition = (opcode >> 3) & 0x3;
    if (condition == 0x00) jump = !isFlagSet(ZERO);
    else if (condition == 0x01) jump = isFlagSet(ZERO);
    else if (condition == 0x02) jump = !isFlagSet(CARRY);
    else if (condition == 0x03) jump = isFlagSet(CARRY);

    if (jump) {
        r.pc += sn;
        return 12;
    } else {
        return 8;
    }
}

u32 CPU::CALL_nn(const u8& opcode) {
    u16 jumpAddress = readWord(r.pc);
    r.pc += 2;
    pushWord(r.pc);
    r.pc = jumpAddress;
    return 24;
}

u32 CPU::CALL_cc_nn(const u8& opcode) {
    u16 nn = readWord(r.pc);
    r.pc += 2;

    bool jump = false;
    u8 condition = (opcode >> 3) & 0x3;
    if (condition == 0x00) jump = !isFlagSet(ZERO);
    else if (condition == 0x01) jump = isFlagSet(ZERO);
    else if (condition == 0x02) jump = !isFlagSet(CARRY);
    else if (condition == 0x03) jump = isFlagSet(CARRY);

    if (jump) {
        pushWord(r.pc);
        r.pc = nn;
        return 24;
    } else {
        return 12;
    }
}

u32 CPU::RST_n(const u8& opcode) {
    u8 destination = ((opcode >> 3) & 0x07) * 0x08;
    pushWord(r.pc);
    r.pc = destination;

    return 16;
}

u32 CPU::RET(const u8& opcode) {
    r.pc = popWord();
    return 16;
}

u32 CPU::RET_cc(const u8& opcode) {
    bool jump = false;
    u8 condition = (opcode >> 3) & 0x03;
    if (condition == 0x00) jump = !isFlagSet(ZERO);
    else if (condition == 0x01) jump = isFlagSet(ZERO);
    else if (condition == 0x02) jump = !isFlagSet(CARRY);
    else if (condition == 0x03) jump = isFlagSet(CARRY);

    if (jump) {
        r.pc = popWord();
        return 20;
    } else {
        return 8;
    }
}

u32 CPU::RETI(const u8& opcode) {
    r.ime = 1;
    r.pc = popWord();
    return 16;
}

// CB Instructions //

u32 CPU::RLC_r(const u8& opcode) {
    u8* reg = byteRegister(opcode);
    isBitSet(*reg, 7) ? setFlag(CARRY) : clearFlag(CARRY);

    (*reg) <<= 1;
    (*reg) = isFlagSet(CARRY) ? setBit(*reg, 0) : clearBit(*reg, 0);

    (*reg == 0) ? setFlag(ZERO) : clearFlag(ZERO);
    clearFlag(ADD_SUB);
    clearFlag(HALF_CARRY);

    return 8;
}

u32 CPU::RLC_HL(const u8& opcode) {
    u8 value = readByte(r.hl);
    isBitSet(value, 7) ? setFlag(CARRY) : clearFlag(CARRY);

    value <<= 1;
    value = isFlagSet(CARRY) ? setBit(value, 0) : clearBit(value, 0);

    (value == 0) ? setFlag(ZERO) : clearFlag(ZERO);
    clearFlag(ADD_SUB);
    clearFlag(HALF_CARRY);

    writeByte(r.hl, value);
    return 16;
}

u32 CPU::RL_r(const u8& opcode) {
    u8* reg = byteRegister(opcode);

    bool oldCarry = isFlagSet(CARRY);
    isBitSet(*reg, 7) ? setFlag(CARRY) : clearFlag(CARRY);

    (*reg) <<= 1;
    (*reg) = oldCarry ? setBit(*reg, 0) : clearBit(*reg, 0);

    (*reg == 0) ? setFlag(ZERO) : clearFlag(ZERO);
    clearFlag(ADD_SUB);
    clearFlag(HALF_CARRY);

    return 8;
}

u32 CPU::RL_HL(const u8& opcode) {
    u8 value = readByte(r.hl);

    bool oldCarry = isFlagSet(CARRY);
    isBitSet(value, 7) ? setFlag(CARRY) : clearFlag(CARRY);

    value <<= 1;
    value = oldCarry ? setBit(value, 0) : clearBit(value, 0);

    (value == 0) ? setFlag(ZERO) : clearFlag(ZERO);
    clearFlag(ADD_SUB);
    clearFlag(HALF_CARRY);

    writeByte(r.hl, value);
    return 16;
}

u32 CPU::RRC_r(const u8& opcode) {
    u8* reg = byteRegister(opcode);

    isBitSet(*reg, 0) ? setFlag(CARRY) : clearFlag(CARRY);
    *reg >>= 1;
    *reg = isFlagSet(CARRY) ? setBit(*reg, 7) : clearBit(*reg, 7);

    (*reg == 0) ? setFlag(ZERO) : clearFlag(ZERO);
    clearFlag(ADD_SUB);
    clearFlag(HALF_CARRY);

    return 8;
}

u32 CPU::RRC_HL(const u8& opcode) {
    u8 value = readByte(r.hl);

    isBitSet(value, 0) ? setFlag(CARRY) : clearFlag(CARRY);
    value >>= 1;
    value = isFlagSet(CARRY) ? setBit(value, 7) : clearBit(value, 7);

    (value == 0) ? setFlag(ZERO) : clearFlag(ZERO);
    clearFlag(ADD_SUB);
    clearFlag(HALF_CARRY);

    writeByte(r.hl, value);
    return 16;
}

u32 CPU::RR_r(const u8& opcode) {
    u8* reg = byteRegister(opcode);

    bool oldCarry = isFlagSet(CARRY);
    isBitSet(*reg, 0) ? setFlag(CARRY) : clearFlag(CARRY);

    *reg >>= 1;
    *reg = oldCarry ? setBit(*reg, 7) : clearBit(*reg, 7);

    (*reg == 0) ? setFlag(ZERO) : clearFlag(ZERO);
    clearFlag(ADD_SUB);
    clearFlag(HALF_CARRY);

    return 8;
}

u32 CPU::RR_HL(const u8& opcode) {
    u8 value = readByte(r.hl);

    bool oldCarry = isFlagSet(CARRY);
    isBitSet(value, 0) ? setFlag(CARRY) : clearFlag(CARRY);

    value >>= 1;
    value = oldCarry ? setBit(value, 7) : clearBit(value, 7);

    (value == 0) ? setFlag(ZERO) : clearFlag(ZERO);
    clearFlag(ADD_SUB);
    clearFlag(HALF_CARRY);

    writeByte(r.hl, value);
    return 16;
}

u32 CPU::SLA_r(const u8& opcode) {
    u8* reg = byteRegister(opcode);
    isBitSet(*reg, 7) ? setFlag(CARRY) : clearFlag(CARRY);

    *reg <<= 1;

    (*reg == 0) ? setFlag(ZERO) : clearFlag(ZERO);
    clearFlag(ADD_SUB);
    clearFlag(HALF_CARRY);

    return 8;
}

u32 CPU::SLA_HL(const u8& opcode) {
    u8 value = readByte(r.hl);
    isBitSet(value, 7) ? setFlag(CARRY) : clearFlag(CARRY);

    value <<= 1;

    (value == 0) ? setFlag(ZERO) : clearFlag(ZERO);
    clearFlag(ADD_SUB);
    clearFlag(HALF_CARRY);

    writeByte(r.hl, value);
    return 16;
}

u32 CPU::SRA_r(const u8& opcode) {
    u8* reg = byteRegister(opcode);
    isBitSet(*reg, 0) ? setFlag(CARRY) : clearFlag(CARRY);

    // the value of bit 7 stays the same
    u8 oldBit7 = *reg & 0x80;
    *reg >>= 1;
    *reg |= oldBit7;

    (*reg == 0) ? setFlag(ZERO) : clearFlag(ZERO);
    clearFlag(ADD_SUB);
    clearFlag(HALF_CARRY);

    return 8;
}

u32 CPU::SRA_HL(const u8& opcode) {
    u8 value = readByte(r.hl);
    isBitSet(value, 0) ? setFlag(CARRY) : clearFlag(CARRY);

    // the value of bit 7 stays the same
    u8 oldBit7 = value & 0x80;
    value >>= 1;
    value |= oldBit7;

    (value == 0) ? setFlag(ZERO) : clearFlag(ZERO);
    clearFlag(ADD_SUB);
    clearFlag(HALF_CARRY);

    writeByte(r.hl, value);
    return 16;
}

u32 CPU::SRL_r(const u8& opcode) {
    u8* reg = byteRegister(opcode);
    isBitSet(*reg, 0) ? setFlag(CARRY) : clearFlag(CARRY);

    // the value of bit 7 is reset
    (*reg) >>= 1;
    (*reg) = clearBit(*reg, 7);

    (*reg == 0) ? setFlag(ZERO) : clearFlag(ZERO);
    clearFlag(ADD_SUB);
    clearFlag(HALF_CARRY);

    return 8;
}

u32 CPU::SRL_HL(const u8& opcode) {
    u8 value = readByte(r.hl);
    isBitSet(value, 0) ? setFlag(CARRY) : clearFlag(CARRY);

    // the value of bit 7 is reset
    value >>= 1;
    value = clearBit(value, 7);

    (value == 0) ? setFlag(ZERO) : clearFlag(ZERO);
    clearFlag(ADD_SUB);
    clearFlag(HALF_CARRY);

    writeByte(r.hl, value);
    return 16;
}

u32 CPU::BIT_b_r(const u8& opcode) {
    u8 bit = (opcode >> 3) & 0x07;
    u8* reg = byteRegister(opcode);

    if (!isBitSet(*reg, bit)) setFlag(ZERO);
    else clearFlag(ZERO);
    setFlag(HALF_CARRY);
    clearFlag(ADD_SUB);

    return 8;
}

u32 CPU::BIT_b_HL(const u8& opcode) {
    u8 bit = (opcode >> 3) & 0x07;
    u8 value = readByte(r.hl);

    (!isBitSet(value, bit)) ? setFlag(ZERO) : clearFlag(ZERO);
    setFlag(HALF_CARRY);
    clearFlag(ADD_SUB);

    return 12;
}

u32 CPU::SET_b_r(const u8& opcode) {
    u8 bitPos = (opcode >> 3) & 0x07;
    u8* reg = byteRegister(opcode);

    *reg = setBit(*reg, bitPos);

    return 8;
}

u32 CPU::SET_b_HL(const u8& opcode) {
    u8 bitPos = (opcode >> 3) & 0x07;
    u8 value = readByte(r.hl);

    value = setBit(value, bitPos);

    writeByte(r.hl, value);
    return 16;
}

u32 CPU::RES_b_r(const u8& opcode) {
    u8 bitPos = (opcode >> 3) & 0x7;
    u8* reg = byteRegister(opcode);

    *reg = clearBit(*reg, bitPos);

    return 8;
}

u32 CPU::RES_b_HL(const u8& opcode) {
    u8 bitPos = (opcode >> 3) & 0x07;
    u8 value = readByte(r.hl);

    value = clearBit(value, bitPos);

    writeByte(r.hl, value);
    return 16;
}

u32 CPU::SWAP_r(const u8& opcode) {
    u8* reg = byteRegister(opcode);
    u8 low = *reg & 0x0F;
    u8 high = *reg & 0xF0;
    *reg = (low << 4) | (high >> 4);

    (*reg == 0x0) ? setFlag(ZERO) : clearFlag(ZERO);
    clearFlag(ADD_SUB);
    clearFlag(HALF_CARRY);
    clearFlag(CARRY);

    return 8;
}

u32 CPU::SWAP_HL(const u8& opcode) {
    u8 value = readByte(r.hl);
    u8 low = value & 0x0F;
    u8 high = value & 0xF0;
    value = (low << 4) | (high >> 4);

    (value == 0) ? setFlag(ZERO) : clearFlag(ZERO);
    clearFlag(ADD_SUB);
    clearFlag(HALF_CARRY);
    clearFlag(CARRY);

    writeByte(r.hl, value);
    return 16;
}
