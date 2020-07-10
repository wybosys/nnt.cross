cmake_minimum_required(VERSION 2.8)
project(tinyxml2)

set(TINYXML2_SRCS
    tinyxml2.cpp)

add_library(tinyxml2 STATIC ${TINYXML2_SRCS})
