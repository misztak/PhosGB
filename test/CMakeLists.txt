file(GLOB phos_TESTS *.cpp *.hpp)

add_executable(test_runner ${phos_TESTS})
target_include_directories(test_runner PUBLIC ../core)
target_link_libraries(test_runner PRIVATE core)
