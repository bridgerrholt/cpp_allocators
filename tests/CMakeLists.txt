set(test_names general_test_0 performance_test_0 unrelated_test_0 unrelated_test_1)

foreach(t ${test_names})
    add_subdirectory(${t})
endforeach(t)