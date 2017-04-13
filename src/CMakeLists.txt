file(GLOB globbed_files
        brh/allocators/*.h
        brh/allocators/blocks/*.h
        brh/allocators/common/*.h
        brh/allocators/wrappers/*.h
)

set(brh_cpp_allocators_source_files ${brh_allocators_source_files} ${globbed_files} PARENT_SCOPE)


