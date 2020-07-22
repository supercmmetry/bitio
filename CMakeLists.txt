cmake_minimum_required(VERSION 3.16)
project(bitio)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories("src")

add_library(bitio src/bitio.h src/bitio.cpp)

enable_testing()
add_subdirectory(tests)

