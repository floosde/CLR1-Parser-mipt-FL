cmake_minimum_required(VERSION 3.0)
project(CLR1_parser)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PARSER_SRC ${CMAKE_SOURCE_DIR}/../sources/CLR1_parser.cpp)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
enable_testing()

add_executable(parser_tests ${PARSER_SRC} parser_tests.cpp)

target_link_libraries(
        parser_tests
        Threads::Threads
        ${GTEST_LIBRARIES}
        ${GMOCK_BOTH_LIBRARIES}
        )

include_directories(${CMAKE_SOURCE_DIR}/../headers)

link_libraries(gtest gtest_main pthread)
