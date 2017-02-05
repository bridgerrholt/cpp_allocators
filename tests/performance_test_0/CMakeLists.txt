project(performance_test_0)

set(source_files get_time.h main.cpp random_instruction_test.h random_size_allocation_test.h test_base.cpp test_base.h)
add_executable(performance_test_0 ${source_files})

target_link_libraries(performance_test_0)