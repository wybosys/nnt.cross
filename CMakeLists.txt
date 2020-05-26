cmake_minimum_required(VERSION 2.8)
project(cross)

set(CMAKE_CXX_STANDARD 11)

include_directories(src)
set(CROSS_SRCS
        src/cross.cpp
        src/fs.cpp
        src/str.cpp
        )
add_library(cross STATIC ${CROSS_SRCS})

set(TEST_SRCS
        test/test.cpp)
add_executable(test ${TEST_SRCS})
target_link_libraries(test cross UnitTest++)
