cmake_minimum_required(VERSION 3.1)

include_directories(../../)

add_library(ajni++ STATIC
    ast.cpp
    android.cpp
    jnienv.cpp
    inspect.cpp
    jre.cpp
    variant.cpp
    kotlin.cpp
    java-prv.cpp
    )

target_link_libraries(ajni++ log)
