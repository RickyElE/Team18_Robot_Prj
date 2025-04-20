cmake_minimum_required(VERSION 3.10)
project(libcam2web)
project(ppr_robotic CXX)

find_package(fastcdr REQUIRED)
find_package(fastrtps REQUIRED)
find_package(OpenCV REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Configuring PPR_ROBOTIC...")
find_package(Threads REQUIRED)

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCAMERA REQUIRED libcamera)
pkg_check_modules(OPENCV REQUIRED opencv4)

# setting the output path, creating the BatteryMsg directory and generate the FastDDS Type Support files
set(BATTERY_DIR ${CMAKE_SOURCE_DIR}/include/BatteryMsg)
set(BATTERY_IDL_FILE ${CMAKE_SOURCE_DIR}/idl/BatteryMsg.idl)

set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

file(MAKE_DIRECTORY ${BATTERY_DIR})

# 添加 libcam2opencv 库的路径和头文件
include_directories(${CMAKE_SOURCE_DIR}/include/libcamera2opencv-master)
link_directories(${CMAKE_SOURCE_DIR}/include/libcamera2opencv-master)

include_directories(${CMAKE_SOURCE_DIR}/include/driver)
include_directories(${CMAKE_SOURCE_DIR}/include/ppr_chassis_node)
include_directories(${CMAKE_SOURCE_DIR}/include/ppr_master_node)
include_directories(${CMAKE_SOURCE_DIR}/include/ppr_remote_node)
include_directories(${CMAKE_SOURCE_DIR}/include/Publisher)
include_directories(${CMAKE_SOURCE_DIR}/include/Subscriber)
include_directories(${CMAKE_SOURCE_DIR}/include/driver/PCA9685)
include_directories(${CMAKE_SOURCE_DIR}/include/driver/DCMotor)
include_directories(${CMAKE_SOURCE_DIR}/include/driver/Ultrasonic)
include_directories(${CMAKE_SOURCE_DIR}/include/driver/SysInfo)
include_directories(${CMAKE_SOURCE_DIR}/include/libcam2_node)
include_directories(${CMAKE_SOURCE_DIR}/include/driver/SerialMotor)
# include_directories(${CMAKE_SOURCE_DIR}/include/driver/SerialMotor/..)
include_directories(${CMAKE_SOURCE_DIR}/include/driver/SerialMotor/Communication)
include_directories(${CMAKE_SOURCE_DIR}/include/driver/SerialMotor/Serial)
include_directories(${CMAKE_SOURCE_DIR}/include/driver/SerialMotor/Control)
include_directories(${CMAKE_SOURCE_DIR}/include/driver/WebArm)
include_directories(${CMAKE_SOURCE_DIR}/include/libcam2_node)

include_directories(
    ${PROJECT_ROOT_DIR}
    /usr/include/opencv4
    /usr/include/aarch64-linux-gnu/opencv4
    ${OpenCV_INCLUDE_DIRS}
    ${LIBCAMERA_INCLUDE_DIRS}
    /usr/include/libcamera
    /usr/include/libcamera/libcamera
    ${CMAKE_SOURCE_DIR}
)
include_directories(/usr/include)

# 創建 libcam2web 庫
set(LIBCAM2WEB_SOURCES
    ${PROJECT_ROOT_DIR}/include/libcam2_node/libcam2web.cpp
    ${PROJECT_ROOT_DIR}/include/libcam2_node/simple_http_server.cpp
    ${PROJECT_ROOT_DIR}/include/libcam2_node/libcam2opencv.cpp
)

# 添加庫
add_library(libcam2web STATIC ${LIBCAM2WEB_SOURCES})

# 連接庫的依賴
target_link_libraries(libcam2web
    ${OpenCV_LIBS}
    ${LIBCAMERA_LIBRARIES}
    camera
    camera-base
    Threads::Threads
    pthread
)

# 其他可執行文件
add_executable(bms ${CMAKE_SOURCE_DIR}/src/test/bms.cpp)

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

include_directories(${CMAKE_SOURCE_DIR}/main)
add_executable(main 
${CMAKE_SOURCE_DIR}/include/driver/PCA9685/PCA9685.cpp 
${CMAKE_SOURCE_DIR}/include/driver/SysInfo/SystemInfo.cpp
${CMAKE_SOURCE_DIR}/include/driver/DCMotor/motor.cpp
${CMAKE_SOURCE_DIR}/include/driver/Ultrasonic/Ultrasonic.cpp
${CMAKE_SOURCE_DIR}/main/main.cpp
${CMAKE_SOURCE_DIR}/include/driver/WebArm/web_arm.cpp
${CMAKE_SOURCE_DIR}/include/driver/SerialMotor/Control/Control.cpp
${CMAKE_SOURCE_DIR}/include/driver/SerialMotor/Serial/Serial.cpp
${CMAKE_SOURCE_DIR}/include/driver/SerialMotor/Communication/Communication.cpp)

# 更新 main 的鏈接庫
target_link_libraries(main 
    PRIVATE gpiod
    libcam2web
    ${OpenCV_LIBS}
    ${LIBCAMERA_LIBRARIES}
)

add_executable(sysInfo_test 
${CMAKE_SOURCE_DIR}/src/test/sysInfo_test.cpp 
${CMAKE_SOURCE_DIR}/include/driver/SysInfo/SystemInfo.cpp)  

install(TARGETS main DESTINATION bin)

add_executable(serialmotor_test 
${CMAKE_SOURCE_DIR}/src/test/serialmotor_test.cpp 
${CMAKE_SOURCE_DIR}/include/driver/SerialMotor/Control/Control.cpp
${CMAKE_SOURCE_DIR}/include/driver/SerialMotor/Serial/Serial.cpp
${CMAKE_SOURCE_DIR}/include/driver/SerialMotor/Communication/Communication.cpp)  