cmake_minimum_required(VERSION 3.0)
project (reactor)

find_package(Threads)
add_library(log_base log.cpp)
add_library(log::library ALIAS log_base)
include_directories(${PROJECT_SOURCE_DIR})

enable_testing()
add_subdirectory(test)