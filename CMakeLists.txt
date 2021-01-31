cmake_minimum_required(VERSION 3.16)
project(bitio LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_library(bitio SHARED src/bitio.cpp)

target_include_directories(bitio
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src)

if (${BITIO_DEVEL})
    enable_testing()
    add_subdirectory(tests)
    add_subdirectory(benchmarks)
endif ()

