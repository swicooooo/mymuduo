cmake_minimum_required(VERSION 3.5)
project(mymuduo)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g") # 设置调试信息

aux_source_directory(. SRC_LIST)
add_library(mymuduo SHARED ${SRC_LIST}) # 生成动态库
