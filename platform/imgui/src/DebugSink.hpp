#ifndef PHOS_DEBUGSINK_HPP
#define PHOS_DEBUGSINK_HPP

#include "imgui.h"
#include "Logger.hpp"

class DebugSink : public Sink {
public:
    DebugSink(bool enabled);
    void printLog(Severity s, const char* message) override;
    void Draw(const char* title, bool* open = nullptr);
private:
    ImGuiTextBuffer Buf;
    ImGuiTextFilter Filter;
    ImVector<int> LineOffsets;
    bool AutoScroll;
    bool ScrollToBottom;

    void Clear();
};

#endif //PHOS_DEBUGSINK_HPP
