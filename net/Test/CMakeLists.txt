add_executable(timer_queue_test timer_queue_test.cpp)
target_link_libraries(timer_queue_test gtest gtest_main net::library base::library)
add_test(timer_queue::test timer_queue_test)