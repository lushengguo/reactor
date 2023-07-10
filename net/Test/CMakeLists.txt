add_library(test_essential INTERFACE)
target_link_libraries(test_essential INTERFACE gtest gtest_main net::library base::library fmt::fmt)

add_executable(timed_task_test timed_task_test.cpp)
target_link_libraries(timed_task_test test_essential)
add_test(timed_task::test timed_task_test)