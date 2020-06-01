cmake_minimum_required(VERSION 2.8)
project(minizip)

set(MINIZIP_SRCS
        ioapi.c
        ioapi_mem.c
        unzip.c)

add_library(minizip STATIC ${MINIZIP_SRCS})
