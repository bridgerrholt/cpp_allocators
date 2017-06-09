set(test_names allocator_containers_test_1
        corruption_test_0
        corruption_test_1
        general_test_0
        multithread_test_0
        performance_test_0
        unrelated_test_0
        unrelated_test_1
        unrelated_test_2)

foreach(t ${test_names})
    add_subdirectory(${t})
endforeach(t)