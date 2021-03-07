add_executable(date_server date_server.cpp)
target_link_libraries(date_server net::library base::library)
add_executable(date_client date_client.cpp)
target_link_libraries(date_client net::library base::library)