#ifndef PHOS_COMMON_HPP
#define PHOS_COMMON_HPP

#include <string>
#include <vector>
#include <cassert>

#include <types.hpp>
#include <bit.hpp>
#include <serializer.hpp>
using namespace hak;

#ifndef __ANDROID__
#include "Logger.hpp"
#define Log(S, MSG, ...) Logger::Log(S, MSG, ##__VA_ARGS__)
#define LogRaw(S, MSG, ...) Logger::LogRaw(S, MSG, ##__VA_ARGS__)
#else
#include <android/log.h>
#define Log(S, ...) __android_log_print(ANDROID_LOG_INFO, "PhosGB", __VA_ARGS__)
#define LogRaw(S, ...) __android_log_print(ANDROID_LOG_INFO, "PhosGB", __VA_ARGS__)
#endif

constexpr u32 WIDTH = 160;
constexpr u32 HEIGHT = 144;
constexpr u32 DISPLAY_TEXTURE_SIZE = WIDTH * HEIGHT * 4;

#endif //PHOS_COMMON_HPP
