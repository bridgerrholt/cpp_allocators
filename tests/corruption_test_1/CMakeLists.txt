project(corruption_test_1)

set(source_files main.cpp run_stack_test.cpp data_handle.h)
add_executable(corruption_test_1 ${source_files})

target_compile_options(corruption_test_1 PUBLIC -O0)

target_link_libraries(corruption_test_1)