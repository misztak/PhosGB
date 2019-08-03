#ifndef PHOS_TIMER_H
#define PHOS_TIMER_H

#include <thread>
#include <chrono>

constexpr int FPS = 60;
constexpr double frameTimeMicro = (1.0 / FPS) * 1e6;

enum TIMER_MODE { ACCURACY, PERFORMANCE, VSYNC};

class Timer {
public:
    Timer(TIMER_MODE mode);
    void setMode(TIMER_MODE newMode);
    TIMER_MODE getMode();
    void toggleMode();
    void syncFrame();
private:
    TIMER_MODE currentMode;
    std::chrono::system_clock::time_point frameStart;
    std::chrono::system_clock::time_point frameEnd;
};

#endif //PHOS_TIMER_H
