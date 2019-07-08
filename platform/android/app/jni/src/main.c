#include "SDL.h"

#include <android/log.h>
#include <jni.h>

#define Log(S, ...) __android_log_print(S, "PhosGB", __VA_ARGS__)

int main(int argc, char* argv[]) {
    // init
    Log(ANDROID_LOG_INFO, "Starting emulator");

    while (1) {
        // emulator loop
    }

    // shutdown

    return 0;
}