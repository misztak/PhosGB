#ifndef PHOS_DEBUGSINK_H
#define PHOS_DEBUGSINK_H

#include "imgui.h"
#include "Logger.h"

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

#endif //PHOS_DEBUGSINK_H
