file(GLOB core_SOURCES *.cpp *.h sound/blip_buf.h sound/blip_buf.c)

add_library(core ${core_SOURCES})
