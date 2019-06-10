#ifndef PHOS_DEBUGGERLOG_H
#define PHOS_DEBUGGERLOG_H

#include "imgui.h"
#include "Logger.h"

class DebuggerLog {
public:
    static ImGuiTextBuffer Buf;
    static ImGuiTextFilter Filter;
    static ImVector<int> LineOffsets;
    static bool AutoScroll;
    static bool ScrollToBottom;

    static void Clear();
    static void DebugLog(Severity s, const char* message);
    static void Draw(const char* title, bool* open = nullptr);
};

#endif //PHOS_DEBUGGERLOG_H
