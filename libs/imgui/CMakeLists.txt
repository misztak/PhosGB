file(GLOB_RECURSE imgui_SOURCES "*.cpp")

find_package(SDL2 REQUIRED)

add_library(imgui ${imgui_SOURCES})

target_include_directories(imgui PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories(imgui PUBLIC ../glew)
target_link_libraries(imgui PRIVATE ${SDL2_LIBRARIES} glew)