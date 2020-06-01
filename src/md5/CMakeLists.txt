cmake_minimum_required(VERSION 2.8)
project(md5)

set(MD5_SRCS
        md5.c)

add_library(md5 STATIC ${MD5_SRCS})
