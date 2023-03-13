add_executable(test_connection test_connection.cpp)
target_link_libraries(test_connection gtest gtest_main net::library base::library)
add_test(socks5_test::connection test_connection)