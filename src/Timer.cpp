#include "Timer.h"

Timer::Timer(TIMER_MODE mode) {
    currentMode = mode;
    switch (currentMode) {
        case ACCURACY:
        case PERFORMANCE: {
            frameStart = std::chrono::system_clock::now();
            frameEnd = std::chrono::system_clock::now();
            break;
        }
        case VSYNC: {
            break;
        }
    }
}

void Timer::setMode(TIMER_MODE newMode) {
    currentMode = newMode;
}

TIMER_MODE Timer::getMode() {
    return currentMode;
}

void Timer::toggleMode() {
    int newMode = (currentMode + 1) % 3;
    currentMode = static_cast<TIMER_MODE>(newMode);
}

void Timer::syncFrame() {
    switch (currentMode) {
        case PERFORMANCE: {
            frameStart = std::chrono::system_clock::now();
            std::chrono::duration<double, std::micro> usedFrameTime = frameStart - frameEnd;
            if (usedFrameTime.count() < frameTimeMicro) {
                std::chrono::duration<double, std::micro> remainingFrameTime(frameTimeMicro - usedFrameTime.count());
                auto remainingFrameTimeLong = std::chrono::duration_cast<std::chrono::microseconds>(remainingFrameTime);
                std::this_thread::sleep_for(std::chrono::microseconds(remainingFrameTimeLong.count()));
            }
            frameEnd = std::chrono::system_clock::now();
            break;
        }
        case ACCURACY: {
            while (true) {
                frameStart = std::chrono::system_clock::now();
                std::chrono::duration<double, std::micro> usedFrameTime = frameStart - frameEnd;
                if (usedFrameTime.count() >= frameTimeMicro) {
                    break;
                }
            }
            frameEnd = frameStart;
            break;
        }
        case VSYNC: {
            break;
        }
    }
}
