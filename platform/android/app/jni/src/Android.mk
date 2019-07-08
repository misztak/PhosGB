LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include

# Add your application source files here...

CORE_PATH := $(LOCAL_PATH)/../../../../../core

CORE_FILES :=   $(CORE_PATH)/sound/blip_buf.c \
                $(CORE_PATH)/APU.cpp \
                $(CORE_PATH)/CPU.cpp \
                $(CORE_PATH)/Emulator.cpp \
                $(CORE_PATH)/GPU.cpp \
                $(CORE_PATH)/Joypad.cpp \
                $(CORE_PATH)/MBC.cpp \
                $(CORE_PATH)/MMU.cpp

MAIN_FILES := $(LOCAL_PATH)/main.c

LOCAL_CFLAGS += -std=c99 -O2
LOCAL_CPPFLAGS += -std=c++17 -O2 -frtti

LOCAL_SRC_FILES := $(CORE_FILES) $(MAIN_FILES)

LOCAL_SHARED_LIBRARIES := SDL2

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog

include $(BUILD_SHARED_LIBRARY)
