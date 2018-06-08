#最低版本要求
cmake_minimum_required(VERSION 2.8)

#指定cmake自定义的查找模块脚本的目录
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

#项目信息
project(dt_cmd C)

#支持IF(A)ELSE()写法
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)

#添加编译选项
ADD_DEFINITIONS(-Wall)
ADD_DEFINITIONS(-Werror)
ADD_DEFINITIONS(-Wunused)
ADD_DEFINITIONS(-std=gnu99)
ADD_DEFINITIONS(-march=native)   #just for dpdk

#指定Debug编译模式("$ cmake -DCMAKE_BUILD_TYPE=Debug ..")
#受困于libev，优化版本需添加参数-fno-strict-aliasing
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Debug)         #开发期间默认为Debug模式
    #set(CMAKE_BUILD_TYPE Release)      #产品最终未Release模式
endif() 
set(CMAKE_C_FLAGS_DEBUG "$ENV{CFLAGS} -O0 -g -ggdb -DMGD_DEBUG")
set(CMAKE_C_FLAGS_RELEASE "$ENV{CFLAGS} -O2 -fno-strict-aliasing")

#指定编译结果生成路径
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

#指定编译依赖
link_directories(
    ${CMAKE_SOURCE_DIR}/deps/lib
)
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/deps/include
)

FILE(GLOB MAIN_SRCS "*.c")

#指定生成目标
add_executable(dt_cmd ${MAIN_SRCS})
target_link_libraries(dt_cmd pthread dl m cjson ev)

