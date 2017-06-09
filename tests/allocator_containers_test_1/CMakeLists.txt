project(allocators_containers_test_1)

set(source_files main.cpp)
add_executable(allocators_containers_test_1 ${source_files})

target_compile_options(allocators_containers_test_1 PUBLIC -O0)

target_link_libraries(allocators_containers_test_1)