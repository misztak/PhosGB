#ifndef PHOS_LOGGER_H
#define PHOS_LOGGER_H

#include <stdarg.h>
#include <cstdio>

#if defined(__clang__) || defined(__GNUC__)
#define FMT_ARGS(FMT)   __attribute__((format(printf, FMT, FMT+1)))
#else
#define IM_FMTARGS(FMT)
#endif

enum Severity { I, D, W, F };

struct Logger {
    static const char* severityStrings[4];

    static bool enabled;
    static size_t counter;

    using LogCallback = void(*)(Severity, const char*);
    static LogCallback callback;

    static void Log(Severity s, const char* message, ...) FMT_ARGS(2);
    static void LogRaw(Severity s, const char* message, ...) FMT_ARGS(2);
};

struct StdLogger {
    static void StdLog(Severity s, const char* message);
};

#endif //PHOS_LOGGER_H
