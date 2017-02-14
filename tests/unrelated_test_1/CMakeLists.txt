project(unrelated_test_1)

set(source_files main.cpp)
add_executable(unrelated_test_1 ${source_files})

target_compile_options(unrelated_test_1 PRIVATE "-O0")