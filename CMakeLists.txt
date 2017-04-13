cmake_minimum_required(VERSION 3.6)
project(brh_cpp_allocators)

set(CMAKE_CXX_STANDARD 14)

option(use_tests "Whether tests should be used or not" ON)

set(allocators_include "${CMAKE_SOURCE_DIR}/src/brh")

#set(Boost_USE_STATIC_LIBS OFF)
#set(Boost_USE_MULTITHREADED ON)
#set(Boost_USE_STATIC_RUNTIME OFF)
#find_package(Boost)

#include_directories(${Boost_INCLUDE_DIRS} ${allocators_include})
include_directories(${allocators_include} cpp_supports/src/brh)

add_subdirectory(cpp_supports)

link_libraries(brh_cpp_supports)

set(brh_cpp_allocators_source_files)
add_subdirectory(src)
#add_library(brh_cpp_allocators ${brh_cpp_allocators_source_files})

if(use_tests)
    set(test_names)
    add_subdirectory(tests)
endif(use_tests)
