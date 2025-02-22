cmake_minimum_required(VERSION 3.10)
project(bms CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)
include_directories(${CMAKE_SOURCE_DIR}/include/ppr_chassis_node)

add_executable(ppr_chassis_node src/ppr_chassis_node/ppr_chassis_node.cpp src/ppr_chassis_node/bms.cpp)