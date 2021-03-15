# add_executable(chat_server chat_server.cpp)
# target_link_libraries(chat_server net::library base::library)

add_executable(chat_client chat_client.cpp protocal.cpp)
target_link_libraries(chat_client net::library base::library)