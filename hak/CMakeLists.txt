set(HAK_SRC
        types.hpp
        bit.hpp)

add_library(hak INTERFACE)

target_include_directories(hak INTERFACE ./)