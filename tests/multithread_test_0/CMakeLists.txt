project(multithread_test_0)

set(source_files main.cpp)
add_executable(multithread_test_0 ${source_files})

target_compile_options(multithread_test_0 PUBLIC -O0)

target_link_libraries(multithread_test_0)