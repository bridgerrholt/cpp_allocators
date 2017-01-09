cmake_minimum_required(VERSION 3.6)
project(cpp_allocator_test)

set(CMAKE_CXX_STANDARD 14)
set(DIRECTORY_BOOST "/Users/bridgerholt/Documents/Programming/C++/_libs/boost_1_62_0/build")
set(BOOST_ROOT "/Users/bridgerholt/Documents/Programming/C++/_libs/boost_1_62_0/build")

include_directories("${DIRECTORY_BOOST}/include")

find_library(LIBBOOST_SYSTEM_LIBRARY libboost_system HINTS ${DIRECTORY_BOOST}/lib)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost 1.62.0 COMPONENTS system)

add_subdirectory("performance_tests")

set(SOURCE_FILES src/main.cpp src/bridgerrholt/allocator_test/allocators/fallback_allocator.h src/bridgerrholt/allocator_test/block.h src/bridgerrholt/allocator_test/block.cpp src/bridgerrholt/allocator_test/common_types.h src/bridgerrholt/allocator_test/allocators/stack_allocator.h src/bridgerrholt/allocator_test/allocator_wrapper.h src/bridgerrholt/allocator_test/allocators/malloc_allocator.h src/bridgerrholt/allocator_test/allocator_singleton.h src/bridgerrholt/allocator_test/allocators/segregator.h src/bridgerrholt/allocator_test/allocators/simple_allocator.h src/bridgerrholt/allocator_test/allocators/segregated_storage.h src/bridgerrholt/allocator_test/allocators/bitmapped_block.h src/bridgerrholt/allocator_test/smart_allocator.h src/bridgerrholt/allocator_test/unique_block.h performance_tests/main.cpp src/bridgerrholt/allocator_test/allocators/free_list.h)
add_executable(cpp_allocator_test ${SOURCE_FILES})

if(UNIX AND NOT APPLE)
    # Linux
    target_link_libraries(cpp_allocator_test pthread dl)
elseif(UNIX AND APPLE)
    # Mac OS
    target_link_libraries(cpp_allocator_test pthread)
else()
    # Windows
    target_link_libraries(cpp_allocator_test)
endif()

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(cpp_allocator_test ${Boost_LIBRARIES})
endif()

target_link_libraries(cpp_allocator_test ${Boost_SYSTEM_LIBRARY})