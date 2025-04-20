cmake_minimum_required(VERSION 3.10)
project(libcam2web)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 查找必要的包
find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCAMERA REQUIRED libcamera)
pkg_check_modules(OPENCV REQUIRED opencv4)

# 设置项目根目录路径
set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 设置包含目录
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

# 添加源文件
set(SOURCES
    ${PROJECT_ROOT_DIR}/main.cpp
    ${PROJECT_ROOT_DIR}/libcam2web.cpp
    ${PROJECT_ROOT_DIR}/simple_http_server.cpp
    ${PROJECT_ROOT_DIR}/libcam2opencv.cpp
)

# 创建可执行文件
add_executable(libcam2web ${SOURCES})

# 链接库
target_link_libraries(libcam2web
    ${OpenCV_LIBS}
    ${LIBCAMERA_LIBRARIES}
    camera
    camera-base
    Threads::Threads
    pthread
)

# 安装目标
install(TARGETS libcam2web DESTINATION bin)
