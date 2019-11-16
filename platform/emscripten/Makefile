#
# Makefile to use with emscripten
# See https://emscripten.org/docs/getting_started/downloads.html
# for installation instructions. This Makefile assumes you have
# loaded emscripten's environment.
#
# Running `make` will produce three files:
#  - phos_emscripten.html
#  - phos_emscripten.js
#  - phos_emscripten.wasm
#

CC = emcc
CXX = em++

EXE = phos_emscripten.html
CORE_DIR = ../../core
CORE_SOURCES = 	$(CORE_DIR)/APU.cpp \
				$(CORE_DIR)/CPU.cpp \
				$(CORE_DIR)/Emulator.cpp \
				$(CORE_DIR)/GPU.cpp \
				$(CORE_DIR)/Joypad.cpp \
				$(CORE_DIR)/Logger.cpp \
				$(CORE_DIR)/MBC.cpp \
				$(CORE_DIR)/MMU.cpp \
				$(CORE_DIR)/sound/blip_buf.c
GUI_DIR = ../imgui/src
GUI_SOURCES = 	$(GUI_DIR)/DebugHost.cpp \
				$(GUI_DIR)/DebugSink.cpp \
				$(GUI_DIR)/Host.cpp \
				$(GUI_DIR)/MainEmscripten.cpp \
				$(GUI_DIR)/NormalHost.cpp
GL3W_SOURCES = 	../imgui/libs/gl3w/GL/gl3w.c
IMGUI_DIR = ../imgui/libs/imgui
IMGUI_SOURCES = $(IMGUI_DIR)/imgui.cpp \
				$(IMGUI_DIR)/imgui_demo.cpp \
				$(IMGUI_DIR)/imgui_draw.cpp \
				$(IMGUI_DIR)/imgui_widgets.cpp \
				$(IMGUI_DIR)/examples/imgui_impl_sdl.cpp \
				$(IMGUI_DIR)/examples/imgui_impl_opengl3.cpp
SOURCES = $(CORE_SOURCES) $(GUI_SOURCES) $(GL3W_SOURCES) $(IMGUI_SOURCES)

OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))
UNAME_S := $(shell uname -s)

EMS = -s USE_SDL=2 -s WASM=1
EMS += -s ALLOW_MEMORY_GROWTH=1
EMS += -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=0
EMS += -s ASSERTIONS=1
# Uncomment next line to fix possible rendering bugs with emscripten version older then 1.39.0 (https://github.com/ocornut/imgui/issues/2877)
EMS += -s BINARYEN_TRAP_MODE=clamp
#EMS += -s NO_FILESYSTEM=1 ## Getting "error: undefined symbol: $FS" if filesystem is removed
#EMS += -s SAFE_HEAP=1    ## Adds overhead

INCLUDES = 	-I../imgui/src/ \
			-I../imgui/libs/imgui/ \
			-I../imgui/libs/gl3w/ \
			-I../imgui/libs/imgui/examples \
			-I../../core/ \
			-I../../core/sound/ \
			-I../../hak/
CPPFLAGS = $(INCLUDES)
#CPPFLAGS += -g
CPPFLAGS += -Wall -Wformat -Os -std=c++17
CPPFLAGS += $(EMS)
CFLAGS = $(INCLUDES) -Wall -Wformat -Os $(EMS)
LIBS = $(EMS)
LDFLAGS = --shell-file shell_minimal.html

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

%.o:../../core/%.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

%.o:../../core/sound/%.c
	$(CC) $(CFLAGS) $(CFLAGS) -c -o $@ $<

%.o:../imgui/src/%.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

%.o:../imgui/libs/imgui/%.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

%.o:../imgui/libs/imgui/examples/%.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

%.o:../imgui/libs/gl3w/GL/%.c
	$(CC) $(CFLAGS) $(CFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(EXE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

clean:
	rm -f $(EXE) $(OBJS) *.js *.wasm *.wasm.pre
