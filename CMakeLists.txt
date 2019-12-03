cmake_minimum_required(VERSION 3.0.0)
project(extended_variant VERSION 0.1.0)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-std=c++17")

set(SOURCES unittests.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})
