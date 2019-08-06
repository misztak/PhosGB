#include <string>
#include <sstream>

#include "Logger.hpp"

const char* Logger::severityStrings[] = {"info","debug","warn","fatal"};

bool Logger::enabled = true;
size_t Logger::counter = 0;
std::vector<std::shared_ptr<Sink>> Logger::sinks;

void Logger::addSink(std::shared_ptr<Sink> sink) {
    sinks.push_back(std::move(sink));
}

void Logger::Log(Severity s, const char* message, ...) {
    if (!enabled)
        return;

    counter++;

    char header[16];
    snprintf(header, 16, "[%05zu][%s]", counter, Logger::severityStrings[s]);

    char buffer[256];
    va_list args;
    va_start(args, message);
    std::vsnprintf(buffer, 256, message, args);
    va_end(args);

    std::ostringstream oss;
    oss << header << " " << buffer;

    for (auto& sink : sinks) {
        if (sink->enabled) sink->printLog(s, oss.str().c_str());
    }
}

void Logger::LogRaw(Severity s, const char* message, ...) {
    if (!enabled)
        return;

    char buffer[256];
    va_list args;
    va_start(args, message);
    std::vsnprintf(buffer, 256, message, args);
    va_end(args);

    for (auto& sink : sinks) {
        if (sink->enabled) sink->printLog(s, buffer);
    }
}

void StdSink::printLog(Severity s, const char* message) {
    if (s == Severity::F)
        printf("\x1b[31m""%s""\x1b[0m", message);
    else
        printf("%s", message);
}
