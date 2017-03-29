project(unrelated_test_2)

set(source_files main.cpp)
add_executable(unrelated_test_2 ${source_files})

target_compile_options(unrelated_test_2 PRIVATE "-O0" "-Wall")