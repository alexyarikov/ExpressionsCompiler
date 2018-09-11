cmake_minimum_required(VERSION 2.8)
project(expression_parser)
find_package(Boost 1.61 REQUIRED)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pedantic")

set(SOURCES expression_parser.cpp expression_evaluator.cpp)
set(HEADERS expression_parser.h expression_evaluator.h)

add_library(expression_parser STATIC ${SOURCES})

include_directories(expression_parser PUBLIC ${Boost_INCLUDE_DIRS})

add_subdirectory(tests)
