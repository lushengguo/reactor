add_executable(test_log test_log.cpp)
target_link_libraries(test_log base::library)

add_executable(test_thread test_thread.cpp)
target_link_libraries(test_thread base::library)

add_executable(test_ThreadPool test_ThreadPool.cpp)
target_link_libraries(test_ThreadPool base::library)