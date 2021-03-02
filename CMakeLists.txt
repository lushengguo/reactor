cmake_minimum_required(VERSION 3.0)
project(reactor)

set(CMAKE_CXX_FLAGS
    " -std=c++2a -g -Wall -Wextra -Werror -Wno-unused-parameter  -Wshadow ")

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(base)
add_subdirectory(net)
add_subdirectory(3rdParty)