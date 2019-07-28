project(ems)
cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXECUTABLE_SUFFIX ".html")

file(GLOB core_SOURCES ../../core/*.cpp ../../core/*.h ../../core/sound/blip_buf.h ../../core/sound/blip_buf.c)
add_executable(phos_asm Main.cpp ${core_SOURCES})

set(EMS_FLAGS "-s USE_SDL=2 -s WASM=0 -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1")
set(CMAKE_C_FLAGS "-Wall -O2 ${EMS_FLAGS}")
set(CMAKE_CXX_FLAGS "-Wall -O2 ${EMS_FLAGS}")

set(CMAKE_EXE_LINKER_FLAGS "-o asm.html")


### in build directory
# emconfigure cmake -DCMAKE_TOOLCHAIN_FILE=$EMSCRIPTEN_ROOT_PATH/cmake/Modules/Platform/Emscripten.cmake ../platform/emscripten
# emmake make
# run from server, e.g. python -m SimpleHTTPServer 8080 in build directory
###
