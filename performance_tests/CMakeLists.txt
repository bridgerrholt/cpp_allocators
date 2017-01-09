cmake_minimum_required(VERSION 3.6)
project(cpp_allocator_test)

set(CMAKE_CXX_STANDARD 14)

include_directories(../src/bridgerrholt)

set(SOURCE_FILES main.cpp ../src/bridgerrholt/allocator_test/block.cpp)
add_executable(allocator_performance_tests ${SOURCE_FILES})

set(CMAKE_CXX_FLAGS "-O0")

target_link_libraries(allocator_performance_tests)