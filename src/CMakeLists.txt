file(GLOB_RECURSE phos_SOURCES "*.cpp")

find_package(SDL2 REQUIRED)

add_executable(phos ${phos_SOURCES})

target_include_directories(phos PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories(phos PUBLIC ../libs/gl3w)
target_include_directories(phos PUBLIC ../libs/imgui)

target_link_libraries(phos PRIVATE imgui gl3w ${SDL2_LIBRARIES})
