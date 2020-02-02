set(TEST_NAME basic_test)

add_subdirectory(stat_lib)
add_subdirectory(dyn_lib)

add_executable(${TEST_NAME} BasicTest.cpp CatchMain.cpp)
target_link_libraries(${TEST_NAME} 
    ll_log 
    $<TARGET_OBJECTS:test_stat>
    test_dyn
)

add_test(NAME tst_basic_test COMMAND  ${TEST_NAME})
