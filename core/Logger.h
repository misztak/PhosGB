#ifndef PHOS_LOGGER_H
#define PHOS_LOGGER_H

#include <stdarg.h>
#include <vector>
#include <memory>

#if defined(__clang__) || defined(__GNUC__)
#define FMT_ARGS(FMT)   __attribute__((format(printf, FMT, FMT+1)))
#else
#define IM_FMTARGS(FMT)
#endif

enum Severity { I, D, W, F };

class Sink {
public:
    bool enabled;
    Sink(bool enabled) : enabled(enabled) {};
    virtual ~Sink() = default;
    virtual void printLog(Severity s, const char* msg) = 0;
};

class StdSink : public Sink {
public:
    StdSink(bool enabled) : Sink(enabled) {};
    void printLog(Severity s, const char* msg) override;
};

struct Logger {
    static const char* severityStrings[4];
    static std::vector<std::shared_ptr<Sink>> sinks;

    static bool enabled;
    static size_t counter;

    static void addSink(std::shared_ptr<Sink>);

    static void Log(Severity s, const char* message, ...) FMT_ARGS(2);
    static void LogRaw(Severity s, const char* message, ...) FMT_ARGS(2);
};

#endif //PHOS_LOGGER_H
