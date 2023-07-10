cmake_minimum_required(VERSION 3.0)
project(reactor)

set(CMAKE_CXX_FLAGS
    " -std=c++2a -g -Wall -Wextra -Werror -Wno-unused-parameter  -Wshadow ")

find_package(Threads REQUIRED)

include_directories(${PROJECT_SOURCE_DIR})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(fmt QUIET)
find_package(GTest QUIET)

if(NOT fmt_FOUND)
  set(FMT_DIR ${CMAKE_BINARY_DIR}/external/fmt)
  include(ExternalProject)
  ExternalProject_Add(
      fmt
      GIT_REPOSITORY https://github.com/fmtlib/fmt.git
      PREFIX ${FMT_DIR}
      INSTALL_COMMAND ""
  )
  include_directories(${FMT_DIR}/src/fmt/include)
  add_library(fmt::fmt ALIAS fmt)
endif()

if(NOT GTest_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
endif()

enable_testing()

add_subdirectory(base)
add_subdirectory(net)
add_subdirectory(examples)