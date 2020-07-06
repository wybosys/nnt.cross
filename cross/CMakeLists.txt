cmake_minimum_required(VERSION 2.8)
project(cross)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

include_directories(.)

set(CROSS_SRCS
    cross.cpp
    fs.cpp
    str.cpp
    connector.cpp
    connector_curl.cpp
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
    )

add_library(cross STATIC ${CROSS_SRCS})
