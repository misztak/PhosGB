#include "SDL.h"

#include <android/log.h>
#include <jni.h>

#include <chrono>
#include <thread>

#include "../../../../../core/Emulator.h"

constexpr double frameTimeMicro = (1.0 / 60) * 1e6;

Emulator emu;
bool shutdown = false;

extern "C" JNIEXPORT void JNICALL
Java_org_phos_phos_PhosActivity_handleInputDown(JNIEnv* env, jobject obj, jint keyCode) {
    emu.handleInputDown(keyCode);
    Log(nullptr, "JNI call to handleInputDown with keyCode %d\n", keyCode);
}

extern "C" JNIEXPORT void JNICALL
Java_org_phos_phos_PhosActivity_handleInputUp(JNIEnv* env, jobject obj, jint keyCode) {
    emu.handleInputUp(keyCode);
    Log(nullptr, "JNI call to handleInputUp with keyCode %d\n", keyCode);
}

std::string getFile() {
    // find the class
    JNIEnv* env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    if (!env) {
        Log(nullptr, "Could not get pointer to JNI environment");
        return std::string();
    }
    jobject sdlActivityInstance = (jobject) SDL_AndroidGetActivity();
    jclass phosActivity = env->GetObjectClass(sdlActivityInstance);
    if (!phosActivity) {
        Log(nullptr, "Could not find PhosActivity class");
        return std::string();
    }
    // find the method
    jmethodID getFilePathMethodId = env->GetMethodID(phosActivity, "getFilePath", "()Ljava/lang/String;");
    if (!getFilePathMethodId) {
        Log(nullptr, "Could not find getFilePath method");
        return std::string();
    }

    // call the method
    jstring result = (jstring) env->CallObjectMethod(sdlActivityInstance, getFilePathMethodId);
    std::string path = env->GetStringUTFChars(result, nullptr);

    //cleanup
    env->DeleteLocalRef(sdlActivityInstance);
    env->DeleteLocalRef(phosActivity);

    return path;
}

void render(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect* viewport) {
    SDL_SetRenderDrawColor(renderer, 0x7F, 0x7F, 0x7F, 0xFF);
    SDL_RenderClear(renderer);

    int pitch = 0;
    unsigned char* displayPtr;
    SDL_LockTexture(texture, nullptr, (void**) &displayPtr, &pitch);
    memcpy(displayPtr, emu.getDisplayState(), 160 * 144 *4);
    SDL_UnlockTexture(texture);
    SDL_RenderCopy(renderer, texture, nullptr, viewport);

    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
    // init
    Log(nullptr, "Initialising SDL");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        Log(nullptr, "Could not init SDL. Error: %s", SDL_GetError());
        return 1;
    }
    int width, height;
    SDL_DisplayMode displayMode;
    if (SDL_GetCurrentDisplayMode(0, &displayMode) == 0) {
        width = displayMode.w;
        height = displayMode.h;
    }

    SDL_Window* window = SDL_CreateWindow("Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        Log(nullptr, "Could not create SDL window. Error: %s", SDL_GetError());
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        Log(nullptr, "Could not create SDL renderer. Error: %s", SDL_GetError());
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);
    if (texture == nullptr) {
        Log(nullptr, "Could not create SDL texture. Error: %s", SDL_GetError());
        return 1;
    }

    SDL_Rect viewport;
    int scale = 6;
    viewport.x = displayMode.w/2 - 160*scale/2, viewport.y = 50;
    viewport.w = 160*scale, viewport.h = 144*scale;

    Log(nullptr, "Successfully created SDL context with window size %dx%d", width, height);
    SDL_SetRenderDrawColor(renderer, 0x7F, 0x7F, 0x7F, 0xFF);

    Log(nullptr, "Starting emulator");

    // call the file chooser
    emu.isHalted = true;
    std::string filePath = getFile();
    if (filePath.empty()) {
        Log(nullptr, "Could not load file %s", filePath.c_str());
    } else {
        emu.load(filePath);
        emu.isHalted = false;
    }

    // init frame sync
    auto frameStart = std::chrono::system_clock::now();
    auto frameEnd = std::chrono::system_clock::now();

    // init audio context
    SDL_AudioSpec spec;
    SDL_zero(spec);
    spec.freq = 44100;
    spec.format = AUDIO_S16;
    spec.channels = 2;
    spec.samples = 4096;
    SDL_AudioDeviceID audio = SDL_OpenAudioDevice(nullptr, 0, &spec, nullptr, 0);
    // TODO: start audio device

    int ticks = 0;
    while (!shutdown) {
        // emulation loop

        if (!emu.isHalted) {
            while (ticks < emu.cpu.ticksPerFrame) {
                int cycles = emu.tick();
                if (cycles == 0) {
                    shutdown = true;
                    break;
                }

                if (emu.hitVBlank()) {
                    render(renderer, texture, &viewport);
                    emu.cpu.apu.readSamples();
//                    if (SDL_GetAudioDeviceStatus(audio) == SDL_AUDIO_PLAYING)
//                        SDL_QueueAudio(audio, emu.cpu.apu.audioBuffer.data(), emu.cpu.apu.audioBuffer.size() * 2);
                }

                ticks += cycles;
            }
            ticks -= emu.cpu.ticksPerFrame;
        }

        // frame sync
        frameStart = std::chrono::system_clock::now();
        std::chrono::duration<double, std::micro> usedFrameTime = frameStart - frameEnd;
        if (usedFrameTime.count() < frameTimeMicro) {
            std::chrono::duration<double, std::micro> remainingFrameTime(frameTimeMicro - usedFrameTime.count());
            auto remainingFrameTimeLong = std::chrono::duration_cast<std::chrono::microseconds>(remainingFrameTime);
            std::this_thread::sleep_for(std::chrono::microseconds(remainingFrameTimeLong.count()));
        }
        frameEnd = std::chrono::system_clock::now();

    }

    // shutdown
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    Log(nullptr, "Shutting down emulator");
    return 0;
}