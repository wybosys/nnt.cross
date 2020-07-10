cmake_minimum_required(VERSION 3.1)
project(tester)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

# 获得项目根目录
get_filename_component(LIB_HOME ../../../ ABSOLUTE)
get_filename_component(LIB_DIR ../../../cross/ ABSOLUTE)

# 添加ajni++项目
add_subdirectory(${LIB_DIR}/ajni++ ajni++)

# 添加cross项目
add_subdirectory(${LIB_DIR} cross)

# 构造测试so
include_directories(${LIB_HOME})
include_directories(${LIB_DIR})
include_directories(${LIB_DIR}/jsoncpp)

add_library(tester SHARED test.cpp ${LIB_DIR}/android_link.cpp)
target_link_libraries(tester cross ajni++ jsoncpp tinyxml2)
