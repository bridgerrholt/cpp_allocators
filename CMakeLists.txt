cmake_minimum_required(VERSION 3.6)
project(cpp_allocator_test)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory("performance_tests")

set(SOURCE_FILES src/main.cpp src/bridgerrholt/allocator_test/allocators/fallback_allocator.h src/bridgerrholt/allocator_test/block.h src/bridgerrholt/allocator_test/block.cpp src/bridgerrholt/allocator_test/common_types.h src/bridgerrholt/allocator_test/allocators/stack_allocator.h src/bridgerrholt/allocator_test/allocator_wrapper.h src/bridgerrholt/allocator_test/allocators/malloc_allocator.h src/bridgerrholt/allocator_test/allocator_singleton.h src/bridgerrholt/allocator_test/allocators/segregator.h src/bridgerrholt/allocator_test/allocators/simple_allocator.h src/bridgerrholt/allocator_test/allocators/full_free_list.h src/bridgerrholt/allocator_test/allocators/bitmapped_block.h src/bridgerrholt/allocator_test/smart_allocator.h src/bridgerrholt/allocator_test/unique_block.h src/bridgerrholt/allocator_test/allocators/free_list.h src/bridgerrholt/allocator_test/allocators/common/free_list_node.h)
add_executable(cpp_allocator_test ${SOURCE_FILES})

target_link_libraries(cpp_allocator_test)