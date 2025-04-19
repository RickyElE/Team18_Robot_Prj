#ifndef LIBCAM2WEB_H
#define LIBCAM2WEB_H

// #include "libcam2opencv.h"
#include "libcam2opencv.h"
#include <memory>
#include <string>
#include <libcamera/libcamera/camera_manager.h>
#include "simple_http_server.h"
#include <opencv4/opencv2/imgcodecs.hpp>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class LibCam2Web {
public:
    LibCam2Web(int port = 8081);
    ~LibCam2Web();  // 添加析構函數聲明
    
    // 啟動攝像頭和網頁服務器
    void start(Libcam2OpenCVSettings settings = Libcam2OpenCVSettings());
    
    // 停止攝像頭和網頁服務器
    void stop();
    
    // 更新當前幀
    void updateFrame(const cv::Mat &frame);
    
private:
    // 服務器回調結構
    struct ServerCallback : Libcam2OpenCV::Callback {
        LibCam2Web* server = nullptr;
        
        virtual void hasFrame(const cv::Mat &frame, const libcamera::ControlList &) {
            if (server != nullptr) {
                server->updateFrame(frame);
            }
        }
    };
    
    // 生成 JPEG 數據
    std::vector<uint8_t> getJpegData();
    
    // 攝像頭和回調
    Libcam2OpenCV camera;
    ServerCallback myCallback;
    
    // 服務器變量
    int port;
    SimpleHttpServer server;
    
    // 幀存儲
    std::mutex frameMutex;
    std::vector<uint8_t> latestFrame;
};

#endif // LIBCAM2WEB_H
