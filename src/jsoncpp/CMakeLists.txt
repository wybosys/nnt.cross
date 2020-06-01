cmake_minimum_required(VERSION 2.8)
project(jsoncpp)

set(JSONCPP_SRCS
        json_reader.cpp
        json_value.cpp
        json_writer.cpp)

add_library(jsoncpp STATIC ${JSONCPP_SRCS})
