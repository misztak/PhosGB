add_library(lodepng lodepng/lodepng.h lodepng/lodepng.cpp)

set(GL3W_SRC gl3w/GL/gl3w.h gl3w/GL/glcorearb.h gl3w/GL/gl3w.c)
add_library(gl3w "${GL3W_SRC}")
target_include_directories(gl3w PUBLIC gl3w)

set(IMGUI_SRC
        imgui/imconfig.h
        imgui/imgui.h
        imgui/imgui.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_internal.h
        imgui/imgui_widgets.cpp
        imgui/imstb_rectpack.h
        imgui/imstb_textedit.h
        imgui/imstb_truetype.h
        imgui/examples/imgui_impl_sdl.h
        imgui/examples/imgui_impl_sdl.cpp
        imgui/examples/imgui_impl_opengl3.h
        imgui/examples/imgui_impl_opengl3.cpp)

find_package(SDL2 CONFIG REQUIRED)
add_library(imgui "${IMGUI_SRC}")
target_include_directories(imgui PUBLIC imgui)
target_include_directories(imgui PUBLIC gl3w)

if (WIN32)
    target_link_libraries(imgui PRIVATE SDL2::SDL2 SDL2::SDL2main gl3w)
else()
    target_include_directories(imgui PUBLIC ${SDL2_INCLUDE_DIRS})
    target_link_libraries(imgui PRIVATE ${SDL2_LIBRARIES} gl3w)
endif()
