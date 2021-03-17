add_executable(redis_client redis_client.cpp protocol.cpp)
target_link_libraries(redis_client net::library base::library)

add_executable(test_redis_client redis_client_test.cpp protocol.cpp)
target_link_libraries(test_redis_client net::library base::library)