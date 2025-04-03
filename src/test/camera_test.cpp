#include <libcamera/libcamera.h>
#include <libcamera/camera_manager.h>
#include <iostream>
#include <memory>

using namespace libcamera;

int main() {
    std::shared_ptr<CameraManager> cm = std::make_shared<CameraManager>();
    cm->start();

    if (cm->cameras().empty()) {
        std::cerr << "No camera found!" << std::endl;
        return -1;
    }

    // 获取第一个摄像头
    std::shared_ptr<Camera> camera = cm->cameras()[0];
    std::cout << "Using camera: " << camera->id() << std::endl;

    // 打开摄像头
    if (camera->acquire()) {
        std::cerr << "Failed to acquire camera" << std::endl;
        return -1;
    }

    if (camera->configure(nullptr)) {
        std::cerr << "Failed to configure camera" << std::endl;
        return -1;
    }

    if (camera->start()) {
        std::cerr << "Failed to start camera" << std::endl;
        return -1;
    }

    std::cout << "Camera started successfully!" << std::endl;

    // TODO: 请求帧、处理帧数据、保存图像

    camera->stop();
    camera->release();
    cm->stop();

    return 0;
}