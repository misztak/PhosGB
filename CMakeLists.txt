cmake_minimum_required(VERSION 3.12)
project(phos)

set(CMAKE_CXX_STANDARD 17)

# compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS}")

# linker flags
if (APPLE)
    set(LIBS "-lc++abi -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo")
elseif(UNIX AND NOT APPLE)
    set(LIBS "-Wl,--start-group -lGL -ldl")
elseif(WIN32)
    # TODO
else()
    message(FATAL_ERROR "Unsupported Platform")
endif ()
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LIBS}")

enable_testing()

add_subdirectory(libs/imgui)
add_subdirectory(libs/glew)
add_subdirectory(src)
add_subdirectory(test)
