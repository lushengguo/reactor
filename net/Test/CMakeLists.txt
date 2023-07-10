add_executable(async_task_test async_task_test.cpp)
target_link_libraries(async_task_test gtest gtest_main net::library base::library)
add_test(timer_queue::test async_task_test)