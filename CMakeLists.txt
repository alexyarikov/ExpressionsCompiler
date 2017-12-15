cmake_minimum_required(VERSION 2.8)
project(expression_compiler)

set(LIB_SOURCES expression_compiler.cpp)
SET(LIB_HEADERS expression_compiler.h)
set(TESTS_SOURCES tests.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -std=c++11 -g")

add_library(expression_compiler STATIC ${LIB_SOURCES})
add_executable(tests ${TESTS_SOURCES})

target_link_libraries(tests expression_compiler)

add_custom_target(cppcheck
						COMMAND /usr/bin/cppcheck --enable=all --std=c++11 --verbose --quiet ${LIB_HEADERS} ${LIB_SOURCES} ${TESTS_SOURCES}
						WORKING_DIRECTORY ..)
