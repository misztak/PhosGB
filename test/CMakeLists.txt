file(GLOB phos_TESTS *.cpp *.hpp)

add_executable(tests ${phos_TESTS})
add_test(main_test tests)

target_include_directories(tests PUBLIC ../src)