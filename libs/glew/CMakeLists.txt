file(GLOB_RECURSE glew_SOURCES "GL/*.c")

add_library(glew ${glew_SOURCES})