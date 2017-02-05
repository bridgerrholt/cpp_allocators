file(GLOB globbed_files
    bridgerrholt/allocators/*.h
    bridgerrholt/allocators/blocks/*.h
    bridgerrholt/allocators/common/*.h
    bridgerrholt/allocators/wrappers/*.h
)

set(brh_cpp_allocators_source_files ${brh_allocators_source_files} ${globbed_files} PARENT_SCOPE)


