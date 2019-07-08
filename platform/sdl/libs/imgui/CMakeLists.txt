add_definitions(-DGLEW_STATIC)

file(GLOB imgui_SOURCES *.cpp *.h)

find_package(SDL2 CONFIG REQUIRED)

add_library(imgui ${imgui_SOURCES})

target_include_directories(imgui PUBLIC ../glew)

if (WIN32)
    target_link_libraries(imgui PRIVATE SDL2::SDL2 SDL2::SDL2main glew)
else()
    target_include_directories(imgui PUBLIC ${SDL2_INCLUDE_DIRS})
    target_link_libraries(imgui PRIVATE ${SDL2_LIBRARIES} glew)
endif()