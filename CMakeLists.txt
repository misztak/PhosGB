cmake_minimum_required(VERSION 3.12)
project(phos)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# compiler flags
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -O2 -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC")
    # release build already includes O3 and DNDEBUG flags (same for c)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
endif()

if (CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -O2")
    set(CMAKE_C_FLAGS_RELEASE  "${CMAKE_C_FLAGS_RELEASE} -Wall")
endif()

# linker flags
if (APPLE)
    set(LIBS "-lc++abi -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo")
elseif(UNIX AND NOT APPLE)
    set(LIBS "-Wl,--start-group -lGL -ldl")
elseif(WIN32)
	set(LIBS "opengl32.lib")
else()
    message(FATAL_ERROR "Unsupported Platform")
endif ()
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LIBS}")

add_subdirectory(hak)
add_subdirectory(platform/imgui)
add_subdirectory(core)
add_subdirectory(test)

#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)