cmake_minimum_required(VERSION 3.0)
project(reactor)

set(CMAKE_CXX_FLAGS
    " -std=c++2a -g -Wall -Wextra -Werror -Wno-unused-parameter  -Wshadow ")

find_package(Threads REQUIRED)

include_directories(${PROJECT_SOURCE_DIR})
set(BINARY_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/bin)

add_subdirectory(base)
add_subdirectory(net)
add_subdirectory(3rdParty)