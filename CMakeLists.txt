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

add_subdirectory(libs/imgui)
add_subdirectory(libs/gl3w)
add_subdirectory(src)