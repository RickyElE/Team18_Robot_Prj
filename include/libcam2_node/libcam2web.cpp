#include "libcam2web.h"
#include <iostream>

// 構造函數
LibCam2Web::LibCam2Web(int port) : port(port), server(port) {
    // 設置回調接收幀
    myCallback.server = this;
    camera.registerCallback(&myCallback);
    
    // 設置圖像提供者
    server.setImageProvider([this]() { return this->getJpegData(); });
}

// 析構函數
LibCam2Web::~LibCam2Web() {
    stop();
}

// 啟動攝像頭和網頁服務器
void LibCam2Web::start(Libcam2OpenCVSettings settings) {
    // 啟動攝像頭
    camera.start(settings);
    
    // 啟動服務器
    if (!server.start()) {
        std::cerr << "啟動網頁服務器失敗" << std::endl;
        camera.stop();
        return;
    }
    
    std::cout << "網頁服務器已在端口 " << port << " 啟動" << std::endl;
    std::cout << "請打開瀏覽器並訪問 http://localhost:" << port << std::endl;
}

// 停止攝像頭和網頁服務器
void LibCam2Web::stop() {
    // 停止服務器
    server.stop();
    
    // 停止攝像頭
    camera.stop();
    
    std::cout << "網頁服務器已停止" << std::endl;
}

// 更新最新的幀
void LibCam2Web::updateFrame(const cv::Mat &frame) {
    std::lock_guard<std::mutex> lock(frameMutex);
    
    // 將幀轉換為JPEG格式
    std::vector<uchar> buf;
    cv::imencode(".jpg", frame, buf, {cv::IMWRITE_JPEG_QUALITY, 90});
    
    // 更新最新的幀數據
    latestFrame.assign(buf.begin(), buf.end());
}

// 生成 JPEG 數據
std::vector<uint8_t> LibCam2Web::getJpegData() {
    std::lock_guard<std::mutex> lock(frameMutex);
    return latestFrame;
}
