cmake_minimum_required(VERSION 3.0)
project(reactor)

set(CMAKE_CXX_FLAGS
    " -std=c++14 -g -Wall -Wextra -Werror -Wno-unused-parameter  -Wshadow ")

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(base)