cmake_minimum_required (VERSION 3.8)

project(whatever-2019)

set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Werror")
add_executable(smoke smoke_test.cpp)
