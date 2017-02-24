project(corruption_test_0)

set(source_files main.cpp generate_instructions.cpp instructions.cpp)
add_executable(corruption_test_0 ${source_files})

target_compile_options(corruption_test_0 PUBLIC -O0)

target_link_libraries(corruption_test_0)