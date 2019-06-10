#include <string>
#include <sstream>

#include "Logger.h"

const char* Logger::severityStrings[] = {"info","debug","warn","fatal"};

bool Logger::enabled = true;
size_t Logger::counter = 0;
Logger::LogCallback Logger::callback = nullptr;

void Logger::Log(Severity s, const char* message, ...) {
    if (!enabled || !callback)
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

    callback(s, oss.str().c_str());
}

void Logger::LogRaw(Severity s, const char *message, ...) {
    if (!enabled || !callback)
        return;

    char buffer[256];
    va_list args;
    va_start(args, message);
    std::vsnprintf(buffer, 256, message, args);
    va_end(args);

    callback(s, buffer);
}

void StdLogger::StdLog(Severity s, const char *message) {
    if (s == Severity::F)
        printf("\x1b[31m""%s""\x1b[0m", message);
    else
        printf("%s", message);
}
