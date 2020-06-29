cmake_minimum_required(VERSION 2.8)
project(cross)

set(CMAKE_CXX_STANDARD 11)

include_directories(cross)
include_directories(cross/jsoncpp)

add_subdirectory(cross/md5)
add_subdirectory(cross/minizip)
add_subdirectory(cross/jsoncpp)

set(CROSS_SRCS
    cross/cross.cpp
    cross/fs.cpp
    cross/str.cpp
    cross/connector.cpp
    cross/connector_curl.cpp
    cross/connector_lws.cpp
    cross/cross.cpp
    cross/datetime.cpp
    cross/digest.cpp
    cross/fs.cpp
    cross/json.cpp
    cross/logger.cpp
    cross/property.cpp
    cross/threads.cpp
    cross/sys.cpp
    cross/url.cpp
    cross/xml.cpp
    cross/zip.cpp
    )
add_library(cross STATIC ${CROSS_SRCS})

set(TEST_SRCS
    test/test.cpp)
add_executable(test ${TEST_SRCS})
target_link_libraries(test cross UnitTest++ tinyxml2 jsoncpp pthread websockets minizip curl md5 z)
