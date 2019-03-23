file(GLOB phos_SOURCES *.cpp *.h)

find_package(SDL2 REQUIRED)

add_executable(phos ${phos_SOURCES})

target_include_directories(phos PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories(phos PUBLIC ../libs/glew)
target_include_directories(phos PUBLIC ../libs/imgui)

target_link_libraries(phos PRIVATE imgui glew ${SDL2_LIBRARIES})
