add_definitions(-DGLEW_STATIC)

file(GLOB phos_SOURCES *.cpp *.h)

find_package(SDL2 CONFIG REQUIRED)

add_executable(phos ${phos_SOURCES})

target_include_directories(phos PUBLIC ../libs/glew)
target_include_directories(phos PUBLIC ../libs/imgui)

if (WIN32)
    target_link_libraries(phos PRIVATE imgui glew SDL2::SDL2 SDL2::SDL2main)
else()
    target_include_directories(phos PUBLIC ${SDL2_INCLUDE_DIRS})
    target_link_libraries(phos PRIVATE imgui glew ${SDL2_LIBRARIES})
endif()

