set(CORE_SOURCES
        sound/blip_buf.h
        sound/blip_buf.c
        APU.hpp
        APU.cpp
        Common.hpp
        CPU.hpp
        CPU.cpp
        Emulator.hpp
        Emulator.cpp
        GPU.hpp
        GPU.cpp
        Joypad.hpp
        Joypad.cpp
        Logger.hpp
        Logger.cpp
        MBC.hpp
        MBC.cpp
        MMU.hpp
        MMU.cpp
        Serializer.hpp)

add_library(core "${CORE_SOURCES}")
