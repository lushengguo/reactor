add_executable(chat_server chat_server.cpp protocol.cpp)
target_link_libraries(chat_server net::library base::library)

add_executable(chat_client chat_client.cpp protocol.cpp)
target_link_libraries(chat_client net::library base::library)