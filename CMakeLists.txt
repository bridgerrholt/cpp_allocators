cmake_minimum_required(VERSION 3.6)
project(cpp_allocator_test)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES src/main.cpp src/bridgerrholt/allocator_test/allocators/fallback_allocator.h src/bridgerrholt/allocator_test/block.h src/bridgerrholt/allocator_test/block.cpp src/bridgerrholt/allocator_test/common_types.h src/bridgerrholt/allocator_test/allocators/stack_allocator.h src/bridgerrholt/allocator_test/round_to_aligned.h src/bridgerrholt/allocator_test/round_to_aligned.cpp)
add_executable(cpp_allocator_test ${SOURCE_FILES})