cmake_minimum_required(VERSION 3.16)
project(bitio_benchmarks)

add_executable(benchmark benchmark.cpp)
target_link_libraries(benchmark bitio pthread profiler)