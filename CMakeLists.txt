cmake_minimum_required(VERSION 3.13)
project(phos)

set(CMAKE_CXX_STANDARD 17)

# compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS}")

# linker flags
if (APPLE)
    set(LIBS "-lc++abi -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo")
elseif(UNIX AND NOT APPLE)
    # TODO
elseif(WIN32)
    # TODO
else()
    message(FATAL_ERROR "Unsupported Platform")
endif ()
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LIBS}")

# include libraries
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(libs/imgui)
include_directories(libs/gl3w)

set(IMGUI
        libs/imgui/imgui.cpp
        libs/imgui/imgui_impl_opengl3.cpp
        libs/imgui/imgui_impl_sdl.cpp
        libs/imgui/imgui_widgets.cpp
        libs/imgui/imgui_draw.cpp
        libs/imgui/imgui_demo.cpp
)
set(GL3W
        libs/gl3w/GL/gl3w.c
)

set(SOURCES main.cpp ${IMGUI} ${GL3W})

add_executable(phos ${SOURCES})
target_link_libraries(phos ${SDL2_LIBRARIES})