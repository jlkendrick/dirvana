cmake_minimum_required(VERSION 3.10)
project(dirvana)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ledit")

add_executable(dirvana main.cpp)