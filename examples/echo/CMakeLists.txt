add_executable(echo_server echo_server.cpp)
target_link_libraries(echo_server net::library base::library)

add_executable(echo_client echo_client.cpp)
target_link_libraries(echo_client net::library base::library)