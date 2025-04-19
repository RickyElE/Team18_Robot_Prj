cmake_minimum_required(VERSION 3.6)
project(ppr_robotic CXX)

find_package(fastcdr REQUIRED)
find_package(fastrtps REQUIRED)

set(CMAKE_CXX_STANDARD 17)

message(STATUS "Configuring PPR_ROBOTIC...")
find_package(Threads REQUIRED)

# setting the output path, creating the BatteryMsg directory and generate the FastDDS Type Support files
set(BATTERY_DIR ${CMAKE_SOURCE_DIR}/include/BatteryMsg)
set(BATTERY_IDL_FILE ${CMAKE_SOURCE_DIR}/idl/BatteryMsg.idl)

file(MAKE_DIRECTORY ${BATTERY_DIR})

add_custom_command(
    OUTPUT  ${BATTERY_DIR}/BatteryMsg.cxx ${BATTERY_DIR}/BatteryMsg.h
            ${BATTERY_DIR}/BatteryMsgPubSubTypes.cxx ${BATTERY_DIR}/BatteryMsgPubSubTypes.h
    COMMAND fastddsgen -replace -d ${BATTERY_DIR} ${BATTERY_IDL_FILE}
    DEPENDS ${BATTERY_IDL_FILE}
    COMMENT "Generating FastDDS Type Support files from IDL"
)

add_custom_target(
    battery_messages
    DEPENDS ${BATTERY_DIR}/BatteryMsg.cxx ${BATTERY_DIR}/BatteryMsgPubSubTypes.cxx
)

include_directories(${BATTERY_DIR})
add_library(BatteryMsg STATIC ${BATTERY_DIR}/BatteryMsg.cxx ${BATTERY_DIR}/BatteryMsgPubSubTypes.cxx)

set_target_properties(BatteryMsg PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${BATTERY_DIR})

add_dependencies(BatteryMsg battery_messages)

include_directories(${CMAKE_SOURCE_DIR}/include/driver)
include_directories(${CMAKE_SOURCE_DIR}/include/ppr_chassis_node)
include_directories(${CMAKE_SOURCE_DIR}/include/ppr_master_node)
include_directories(${CMAKE_SOURCE_DIR}/include/ppr_remote_node)
include_directories(${CMAKE_SOURCE_DIR}/include/Publisher)
include_directories(${CMAKE_SOURCE_DIR}/include/Subscriber)
include_directories(${CMAKE_SOURCE_DIR}/include/driver/PCA9685)
include_directories(${CMAKE_SOURCE_DIR}/include/driver/DCMotor)
include_directories(${CMAKE_SOURCE_DIR}/include/driver/Ultrasonic)
include_directories(${CMAKE_SOURCE_DIR}/include/driver/Servo)
include_directories(${CMAKE_SOURCE_DIR}/include/driver/SerialMotor)
include_directories(${CMAKE_SOURCE_DIR}/include/driver/SerialMotor/Communication)
include_directories(${CMAKE_SOURCE_DIR}/include/driver/SerialMotor/Control)
include_directories(${CMAKE_SOURCE_DIR}/include/driver/SerialMotor/Serial)
include_directories(${CMAKE_SOURCE_DIR}/include/SCServo_Linux_220329)
include_directories(${CMAKE_SOURCE_DIR}/src/test)


# add_custom_target(
#     messages
#     COMMAND fastddsgen ${CMAKE_SOURCE_DIR}/idl/ChassisMsg.idl
# )

# add_library(ChassisMsg STATIC ${CMAKE_SOURCE_DIR}/src/driver/ChassisMsg.cxx ${CMAKE_SOURCE_DIR}/src/driver/ChassisMsgPubSubType.cxx)
# add_dependencies(ChassisMsg messages)

# add_executable(ppr_chassis_node src/ppr_chassis_node/ppr_chassis_node.cpp src/driver/bms.cpp)
add_executable(bms ${CMAKE_SOURCE_DIR}/src/test/bms.cpp)

add_executable(Test_WritePos ${CMAKE_SOURCE_DIR}/src/test/Test_WritePos.cpp
                        ${CMAKE_SOURCE_DIR}/include/driver/Servo/RoboArm.cpp
                        ${CMAKE_SOURCE_DIR}/include/driver/SerialMotor/Control/Control.cpp
                        ${CMAKE_SOURCE_DIR}/include/driver/SerialMotor/Communication/Communication.cpp
                        ${CMAKE_SOURCE_DIR}/include/driver/SerialMotor/Serial/Serial.cpp
                        ${CMAKE_SOURCE_DIR}/include/driver/Servo/UserInput.cpp)
                        
add_executable(delay ${CMAKE_SOURCE_DIR}/src/test/delay.cpp)

add_executable(pwm_test ${CMAKE_SOURCE_DIR}/src/test/pwm_test.cpp ${CMAKE_SOURCE_DIR}/include/driver/PCA9685/PCA9685.cpp)

add_executable(motor_test ${CMAKE_SOURCE_DIR}/src/test/motor_test.cpp 
                            ${CMAKE_SOURCE_DIR}/include/driver/PCA9685/PCA9685.cpp 
                            ${CMAKE_SOURCE_DIR}/include/driver/DCMotor/motor.cpp)

add_executable(ultrasonic_test ${CMAKE_SOURCE_DIR}/src/test/ultrasonic_test.cpp 
                            ${CMAKE_SOURCE_DIR}/include/driver/Ultrasonic/Ultrasonic.cpp)
target_link_libraries(ultrasonic_test PRIVATE gpiod)

add_executable(Chassis_test ${CMAKE_SOURCE_DIR}/src/test/Chassis_test.cpp 
                            ${CMAKE_SOURCE_DIR}/include/driver/PCA9685/PCA9685.cpp 
                            ${CMAKE_SOURCE_DIR}/include/driver/DCMotor/motor.cpp
                            ${CMAKE_SOURCE_DIR}/include/driver/Ultrasonic/Ultrasonic.cpp)
target_link_libraries(Chassis_test PRIVATE gpiod)

add_executable(test_sysinfo ${CMAKE_SOURCE_DIR}/src/test/test_sysinfo.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCAMERA REQUIRED libcamera)
include_directories(${LIBCAMERA_INCLUDE_DIRS})
link_directories(${LIBCAMERA_LIBRARY_DIRS})
add_executable(camera_test ${CMAKE_SOURCE_DIR}/src/test/camera_test.cpp)
target_link_libraries(camera_test ${LIBCAMERA_LIBRARIES})

add_executable(web_arm  ${CMAKE_SOURCE_DIR}/src/test/web_arm.cpp
                        ${CMAKE_SOURCE_DIR}/include/driver/Servo/RoboArm.cpp
                        ${CMAKE_SOURCE_DIR}/include/driver/Servo/SCS.cpp
                        ${CMAKE_SOURCE_DIR}/include/driver/Servo/SCSCL.cpp
                        ${CMAKE_SOURCE_DIR}/include/driver/Servo/SCSerial.cpp
                        ${CMAKE_SOURCE_DIR}/include/driver/Servo/SMS_STS.cpp
                        ${CMAKE_SOURCE_DIR}/include/driver/PCA9685/PCA9685.cpp
                        ${CMAKE_SOURCE_DIR}/include/driver/Servo/UserInput.cpp)


add_executable(jiazhua  ${CMAKE_SOURCE_DIR}/src/test/jiazhua.cpp 
                        ${CMAKE_SOURCE_DIR}/include/driver/PCA9685/PCA9685.cpp )
                        


                        