cmake_minimum_required(VERSION 3.16)
project(CLR1_parser)

set(CMAKE_CXX_STANDARD 20)
include_directories(headers)
add_executable(CLR1_parser main.cpp sources/CLR1_parser.cpp)
