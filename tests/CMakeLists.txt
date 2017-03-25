set(test_names general_test_0 performance_test_0 unrelated_test_0 unrelated_test_1 corruption_test_0 corruption_test_1 multithread_test_0)

foreach(t ${test_names})
    add_subdirectory(${t})
endforeach(t)