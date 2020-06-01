cmake_minimum_required(VERSION 2.8)
project(cross)

set(CMAKE_CXX_STANDARD 11)

include_directories(src)
add_subdirectory(src/md5)
add_subdirectory(src/minizip)
add_subdirectory(src/jsoncpp)

set(CROSS_SRCS
        src/cross.cpp
        src/fs.cpp
        src/str.cpp
        src/connector.cpp
        src/connector_curl.cpp
        src/connector_lws.cpp
        src/cross.cpp
        src/datetime.cpp
        src/digest.cpp
        src/fs.cpp
        src/json.cpp
        src/logger.cpp
        src/property.cpp
        src/threads.cpp
        src/url.cpp
        src/xml.cpp
        src/zip.cpp
        )
add_library(cross STATIC ${CROSS_SRCS})

set(TEST_SRCS
        test/test.cpp)
add_executable(test ${TEST_SRCS})
target_link_libraries(test cross UnitTest++ tinyxml2 jsoncpp pthread websockets minizip curl md5 z)
