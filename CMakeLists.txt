cmake_minimum_required(VERSION 3.10)
project(ppr_robotic CXX)

find_package(fastcdr REQUIRED)
find_package(fastrtps REQUIRED)
set(CMAKE_CXX_STANDARD 17)

message(STATUS "Configuring PPR_ROBOTIC...")
find_package(Threads REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include/driver)
include_directories(${CMAKE_SOURCE_DIR}/include/ppr_chassis_node)
include_directories(${CMAKE_SOURCE_DIR}/include/ppr_master_node)
add_custom_target(
    messages
    COMMAND fastddsgen ${CMAKE_SOURCE_DIR}/idl/ChassisMsg.idl
)

add_library(ChassisMsg STATIC ${CMAKE_SOURCE_DIR}/src/driver/ChassisMsg.cxx ${CMAKE_SOURCE_DIR}/src/driver/ChassisMsgPubSubType.cxx)
add_dependencies(ChassisMsg messages)

# add_executable(ppr_chassis_node src/ppr_chassis_node/ppr_chassis_node.cpp src/driver/bms.cpp)
add_executable(ppr_chassis_node ${CMAKE_SOURCE_DIR}/src/ppr_chassis_node/ppr_chassis_node.cpp)
target_link_libraries(ppr_chassis_node ChassisMsg fastcdr fastrtps)

add_executable(ppr_master_node ${CMAKE_SOURCE_DIR}/src/ppr_master_node/ppr_master_node.cpp)
target_link_libraries(ppr_master_node ChassisMsg fastcdr fastrtps)

add_executable(bms ${CMAKE_SOURCE_DIR}/src/driver/bms.cpp)