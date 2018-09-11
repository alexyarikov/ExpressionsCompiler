cmake_minimum_required(VERSION 2.8)
project(expression_parser_tests)
find_package(GTest REQUIRED)

set(SOURCES main.cpp)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ..)
add_executable(tests ${SOURCES})
target_link_libraries(tests gtest gtest_main pthread expression_parser)
