#pragma once

#include <types.hpp>

namespace hak {
    template<typename T> constexpr inline T setBit(T value, u8 bitPos) {
        return value | (1 << bitPos);
    }

    template<typename T> constexpr inline T clearBit(T value, u8 bitPos) {
        return value & ~(1 << bitPos);
    }

    template<typename T> constexpr inline bool isBitSet(T value, u8 bitPos) {
        return value & (1 << bitPos);
    }
}