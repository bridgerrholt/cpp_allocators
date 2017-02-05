set(test_names general_test_0 performance_test_0)

foreach(t ${test_names})
    add_subdirectory(${t})
endforeach(t)