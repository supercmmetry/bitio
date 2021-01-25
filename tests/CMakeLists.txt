project(bitio_test)

add_subdirectory(googletest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(bitio_test bitio.cpp)
target_link_libraries(bitio_test gtest gtest_main bitio)