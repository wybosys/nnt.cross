cmake_minimum_required(VERSION 2.8)
project(cross)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

add_subdirectory(md5)
add_subdirectory(minizip)
add_subdirectory(jsoncpp)
add_subdirectory(tinyxml2)

include_directories(.)
include_directories(jsoncpp)

set(CROSS_SRCS
    cross.cpp
    fs.cpp
    str.cpp
    connector.cpp
    connector_jni.cpp
    cross.cpp
    datetime.cpp
    digest.cpp
    fs.cpp
    json.cpp
    logger.cpp
    property.cpp
    threads.cpp
    timer.cpp
    sys.cpp
    url.cpp
    xml.cpp
    zip.cpp
    android.cpp
    )

add_library(cross STATIC ${CROSS_SRCS})
