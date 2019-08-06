set(CORE_SOURCES
        sound/blip_buf.h
        sound/blip_buf.c
        APU.h
        APU.cpp
        Common.h
        CPU.h
        CPU.cpp
        Emulator.h
        Emulator.cpp
        GPU.h
        GPU.cpp
        Joypad.h
        Joypad.cpp
        Logger.h
        Logger.cpp
        MBC.h
        MBC.cpp
        MMU.h
        MMU.cpp
        Serializer.h)

add_library(core "${CORE_SOURCES}")
