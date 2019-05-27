#include "APU.h"
#include "CPU.h"

Channel::Channel(CPU *cpu) : cpu(cpu) {}

u8 Channel::getReg(u8 address) {
    return cpu->mmu.IO[address];
}

Square1Channel::Square1Channel(CPU* cpu) : Channel(cpu) {}

void Square1Channel::reset() {
    on = true; timer = 1;
    envelopeSweeps = getReg(CH1_VOL_ENVELOPE) & 0x7;
    sweepLength = (getReg(CH1_SWEEP) >> 4) & 0x7;
    sweepOn = sweepLength || getReg(CH1_SWEEP) & 0x7;
    sweepFreqency = ((getReg(CH1_FREQ_HIGH) & 0x7) << 8) | getReg(CH1_FREQ_LOW);
    if (lengthCounter == 0) lengthCounter = 0x3F;
    volume = getReg(CH1_VOL_ENVELOPE) >> 4;
}

void Square1Channel::updateFrame(u8 frameStep) {
    sweepLength--;
    if (lengthCounter == 1 && isBitSet(frameStep, 0x01) && isBitSet(getReg(CH1_FREQ_HIGH), 0x40)) {
        on = false; lengthCounter = 0;
    }
    if (frameStep == 7 && envelopeSweeps == 1) {
        envelopeSweeps = 0;
        if (isBitSet(getReg(CH1_VOL_ENVELOPE), 0x08) && volume < 0xF) volume++;
        else if (!isBitSet(getReg(CH1_VOL_ENVELOPE), 0x08) && volume > 0x0) volume--;
        envelopeSweeps = getReg(CH1_VOL_ENVELOPE) & 0x7;
    }
}

void Square1Channel::updateWave() {
    timer = ((0x800 - (((getReg(CH1_FREQ_HIGH) & 0x7) << 8) | getReg(CH1_FREQ_LOW))) << 1) + 1;
    u8 dutyPattern = getReg(CH1_SOUND_LENGTH) >> 6;
    waveStep = (waveStep + 1) & 0x7;
    channelOutput = 0;
    if (on && isBitSet(dutyCycleWaveform[dutyPattern], 0x1 << waveStep)) channelOutput = volume;
}

Square2Channel::Square2Channel(CPU* cpu) : Channel(cpu) {}

void Square2Channel::reset() {
    on = true; timer = 1;
    envelopeSweeps = getReg(CH2_VOL_ENVELOPE) & 0x7;
    if (lengthCounter == 0) lengthCounter = 0x3F;
    volume = getReg(CH2_VOL_ENVELOPE) >> 4;
}

void Square2Channel::updateFrame(u8 frameStep) {
    if (lengthCounter == 1 && isBitSet(frameStep, 0x01) && isBitSet(getReg(CH2_FREQ_HIGH), 0x40)) {
        on = false; lengthCounter = 0;
    }
    if (frameStep == 7 && envelopeSweeps == 1) {
        envelopeSweeps = 0;
        if (isBitSet(getReg(CH2_VOL_ENVELOPE), 0x08) && volume < 0xF) volume++;
        else if (!isBitSet(getReg(CH2_VOL_ENVELOPE), 0x08) && volume > 0x0) volume--;
        envelopeSweeps = getReg(CH2_VOL_ENVELOPE) & 0x7;
    }
}

void Square2Channel::updateWave() {
    timer = ((0x800 - (((getReg(CH2_FREQ_HIGH) & 0x7) << 8) | getReg(CH2_FREQ_LOW))) << 1) + 1;
    u8 dutyPattern = getReg(CH2_SOUND_LENGTH) >> 6;
    waveStep = (waveStep + 1) & 0x7;
    channelOutput = 0;
    if (on && isBitSet(dutyCycleWaveform[dutyPattern], 0x1 << waveStep)) channelOutput = volume;
}

WaveChannel::WaveChannel(CPU *cpu) : Channel(cpu) {}

void WaveChannel::reset() {
    on = true; timer = 1, envelopeSweeps = 0, waveStep = 0;
    if (lengthCounter == 0) lengthCounter = 0xFF;
}

void WaveChannel::updateFrame(u8 frameStep) {
    if (lengthCounter == 1 && isBitSet(frameStep, 0x01) && isBitSet(getReg(CH3_FREQ_HIGH), 0x40)) {
        on = false; lengthCounter = 0;
    }
}

void WaveChannel::updateWave() {
    timer = ((0x800 - (((getReg(CH3_FREQ_HIGH) & 0x7) << 8) | getReg(CH3_FREQ_LOW))) << 1) + 1;
    waveStep = (waveStep + 1) & 0x1F;
    u8 waveformData = cpu->mmu.IO[0x30 + (waveStep >> 1)];
    if (isBitSet(waveStep, 0x01)) waveformData >>= 4;
    else waveformData &= 0xF;
    channelOutput = 0;
    if (on) channelOutput = waveformData >> volume;
}

NoiseChannel::NoiseChannel(CPU *cpu) : Channel(cpu) {}

void NoiseChannel::reset() {
    on = false; timer = 1, lsfr = 0xFF;
    envelopeSweeps = getReg(CH4_VOL_ENVELOPE) & 0x7;
    if (lengthCounter == 0) lengthCounter = 0x3F;
    volume = getReg(CH4_VOL_ENVELOPE) >> 4;
}

void NoiseChannel::updateFrame(u8 frameStep) {
    if (lengthCounter == 1 && isBitSet(frameStep, 0x01) && isBitSet(getReg(CH4_COUNTER_INITIAL), 0x40)) {
        on = false; lengthCounter = 0;
    }
    if (frameStep == 7 && envelopeSweeps == 1) {
        envelopeSweeps = 0;
        if (isBitSet(getReg(CH4_VOL_ENVELOPE), 0x08) && volume < 0xF) volume++;
        else if (!isBitSet(getReg(CH4_VOL_ENVELOPE), 0x08) && volume > 0x0) volume--;
        envelopeSweeps = getReg(CH4_VOL_ENVELOPE) & 0x7;
    }
}

void NoiseChannel::updateWave() {
    bool lowBits = isBitSet(lsfr, 0x01) ^ isBitSet(lsfr, 0x02);
    timer = ((noiseDivisors[getReg(CH4_POLY_COUNTER) & 0x7] / 2) << (getReg(CH4_POLY_COUNTER) >> 4)) + 1;
    if (lowBits) lsfr = ((lsfr >> 1) | (0x1 << 14));
    else lsfr = ((lsfr >> 1) & ~(0x1 << 14));
    if (isBitSet(getReg(CH4_POLY_COUNTER), 0x08)) {
        if (lowBits) lsfr = (lsfr | (0x1 << 6));
        else lsfr = (lsfr & ~(0x1 << 6));
    }
    channelOutput = 0;
    if (on && !isBitSet(lsfr, 0x01)) channelOutput = volume;
}



APU::APU(CPU* cpu) : cpu(cpu), ch1(cpu), ch2(cpu), ch3(cpu), ch4(cpu) {
    channels[0] = &ch1;
    channels[1] = &ch2;
    channels[2] = &ch3;
    channels[3] = &ch4;

    // match divider timer
    left_buffer = blip_new(16383);
    right_buffer = blip_new(16383);
    // downsample
    blip_set_rates(left_buffer, 2097152, 44200);
    blip_set_rates(right_buffer, 2097152, 44200);
}

APU::~APU() {
    blip_delete(left_buffer);
    blip_delete(right_buffer);
}

void APU::update(u32 cycles) {
    u32 mCycles = cycles / 4;
    bool dividerCycle = isBitSet(cpu->mmu.IO[0x04], 0x10);
    if (lastCounter && !dividerCycle) {
        frame = (frame + 1) & 0x7;
        for (auto& channel : channels) {
            channel->updateFrame(frame);
        }
    }
    lastCounter = dividerCycle;
    for (size_t i = 0; i < mCycles * 2; i++) {
        if (++sample == 0) {
            blip_end_frame(left_buffer, 0xFF);
            blip_end_frame(right_buffer, 0xFF);
        }

        short deltaLeft = 0, deltaRight = 0;
        for (auto& channel : channels) {
            if (--channel->timer != 0)
                continue;
            channel->updateWave();
            short delta = channel->channelOutput - channel->lastOutput;
            channel->lastOutput += delta;
            if (channel->onLeft)
                deltaLeft += delta;
            if (channel->onRight)
                deltaRight += delta;
        }
        if (deltaLeft != 0)
            blip_add_delta_fast(left_buffer, sample, deltaLeft * 128);
        if (deltaRight != 0)
            blip_add_delta_fast(right_buffer, sample, deltaRight * 128);
    }
}

void APU::readSamples() {
    int size = blip_samples_avail(right_buffer);
    audioBuffer.resize(size * 2);
    blip_read_samples(left_buffer, audioBuffer.data(), size, true);
    blip_read_samples(right_buffer, audioBuffer.data() + 1, size, true);
}