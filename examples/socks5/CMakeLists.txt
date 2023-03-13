add_executable(echo_socks5_server echo_server.cpp)
target_link_libraries(echo_socks5_server net::library base::library)

add_executable(echo_socks5_client echo_client.cpp)
target_link_libraries(echo_socks5_client net::library base::library)

add_executable(socks5_server socks5_server.cpp)
target_link_libraries(socks5_server net::library base::library)

add_subdirectory(Test)