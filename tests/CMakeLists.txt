project(Google_tests)

add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(gtest_run bitio.cpp)
target_link_libraries(gtest_run gtest gtest_main bitio)