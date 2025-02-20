cmake_minimum_required(VERSION 3.10)
project(bms CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)
include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(ppr_chassis_node src/ppr_chassis_node.cpp src/bms.cpp)