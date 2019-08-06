add_definitions(-DGLEW_STATIC)

add_subdirectory(libs)

set(PHOS_SOURCES
        src/DebugHost.hpp
        src/DebugHost.cpp
        src/DebugSink.hpp
        src/DebugSink.cpp
        src/FileBrowser.hpp
        src/MemoryEditor.hpp
        src/Host.hpp
        src/Host.cpp
        src/Main.cpp
        src/NormalHost.hpp
        src/NormalHost.cpp
        src/Timer.hpp
        src/Timer.cpp)

find_package(SDL2 CONFIG REQUIRED)

add_executable(phos "${PHOS_SOURCES}")

target_include_directories(phos PUBLIC ../../core)
target_include_directories(phos PUBLIC libs/gl3w)
target_include_directories(phos PUBLIC libs/imgui)
target_include_directories(phos PUBLIC libs/imgui/examples)
target_include_directories(phos PUBLIC libs/lodepng)

if (WIN32)
    target_link_libraries(phos PRIVATE core imgui gl3w SDL2::SDL2 SDL2::SDL2main lodepng)
else()
    target_include_directories(phos PUBLIC ${SDL2_INCLUDE_DIRS})
    target_link_libraries(phos PRIVATE core imgui gl3w ${SDL2_LIBRARIES} lodepng)
endif()

