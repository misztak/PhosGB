add_definitions(-DGLEW_STATIC)

file(GLOB glew_SOURCES GL/*.c GL/*.h)

add_library(glew ${glew_SOURCES})