#ifndef PHOS_APU_H
#define PHOS_APU_H

#include "Common.h"
#include "sound/blip_buf.h"

// Channel 1
constexpr u16 CH1_SWEEP             = 0x10;
constexpr u16 CH1_SOUND_LENGTH      = 0x11;
constexpr u16 CH1_VOL_ENVELOPE      = 0x12;
constexpr u16 CH1_FREQ_LOW          = 0x13;
constexpr u16 CH1_FREQ_HIGH         = 0x14;
// Channel 2
constexpr u16 CH2_SOUND_LENGTH      = 0x16;
constexpr u16 CH2_VOL_ENVELOPE      = 0x17;
constexpr u16 CH2_FREQ_LOW          = 0x18;
constexpr u16 CH2_FREQ_HIGH         = 0x19;
// Channel 3
constexpr u16 CH3_SOUND_ON_OFF      = 0x1A;
constexpr u16 CH3_SOUND_LENGTH      = 0x1B;
constexpr u16 CH3_OUTPUT_LVL_SELECT = 0x1C;
constexpr u16 CH3_FREQ_LOW          = 0x1D;
constexpr u16 CH3_FREQ_HIGH         = 0x1E;
// Channel 4
constexpr u16 CH4_SOUND_LENGTH      = 0x20;
constexpr u16 CH4_VOL_ENVELOPE      = 0x21;
constexpr u16 CH4_POLY_COUNTER      = 0x22;
constexpr u16 CH4_COUNTER_INITIAL   = 0x23;

constexpr u16 CHANNEL_CONTROL       = 0x24;
constexpr u16 OUTPUT_SELECT         = 0x25;
constexpr u16 SOUND_ON_OFF          = 0x26;

class CPU;

class Channel {
public:
    Channel(CPU* cpu);
    virtual void reset() = 0;
    virtual void updateFrame(u8 frameStep) = 0;
    virtual void updateWave() = 0;
    void updateLengthCounter(u8 channel, u8 frameStep);
    u8 getReg(u8 address);
    void setReg(u8 address, u8 value);
    void hardReset();
public:
    CPU* cpu;

    bool on = false, onRight = true, onLeft = true;
    u8 channelOutput = 0, lastOutput = 0, volume = 0, waveStep = 0;
    u16 timer = 0, lengthCounter = 0, envelopeSweeps = 0;

    static constexpr u8 dutyCycleWaveform[4] = { 0b00000001, 0b10000001, 0b10000111, 0b01111110 };
    static constexpr u8 volumeShifts[4] = { 4, 0, 1, 2 };
};

class Square1Channel : public Channel {
public:
    Square1Channel(CPU* cpu);
    void reset() override;
    void updateFrame(u8 frameStep) override;
    void updateWave() override;
public:
    bool sweepOn = false;
    u16 sweepFrequency = 0, sweepLength = 0;
};

class Square2Channel : public Channel {
public:
    Square2Channel(CPU* cpu);
    void reset() override;
    void updateFrame(u8 frameStep) override;
    void updateWave() override;
};

class WaveChannel : public Channel {
public:
    WaveChannel(CPU* cpu);
    void reset() override;
    void updateFrame(u8 frameStep) override;
    void updateWave() override;
};

class NoiseChannel : public Channel {
public:
    NoiseChannel(CPU* cpu);
    void reset() override;
    void updateFrame(u8 frameStep) override;
    void updateWave() override;
public:
    u16 lsfr = 0xFF;
private:
    static constexpr u8 noiseDivisors[8] = { 8, 16, 32, 48, 64, 80, 96, 112 };
};

class APU {
public:
    APU(CPU* cpu);
    ~APU();
    void update(u32 cycles);
    void readSamples();

    void reset();
    void saveState(std::ofstream& outfile);
private:
    CPU* cpu;

    bool lastCounter = false;
    u8 sample = 0;
    u8 frame = 0;

    blip_t *left_buffer, *right_buffer;
public:
    std::vector<short> audioBuffer;
    Square1Channel ch1;
    Square2Channel ch2;
    WaveChannel ch3;
    NoiseChannel ch4;
    Channel* channels[4] = {nullptr};
};

#endif //PHOS_APU_H
