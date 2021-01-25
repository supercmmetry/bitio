cmake_minimum_required(VERSION 3.16)
project(bitio_benchmarks)

add_executable(bitio_benchmarks benchmark.cpp)
target_link_libraries(bitio_benchmarks bitio pthread profiler)