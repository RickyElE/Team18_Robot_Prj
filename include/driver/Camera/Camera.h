#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "libcam2opencv.h"
#include "opencv2/imgcodecs.hpp"
#include <websocketpp/common/connection_hdl.hpp>
#include <websocketpp/common/system_error.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/frame.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp> 
#include <atomic>
#include <thread>
#include <chrono>
#include <mutex>
#include <functional>
#include <websocketpp/transport/base/endpoint.hpp>

typedef websocketpp::server<websocketpp::config::asio> websocket_server;

class Camera{
public:
    Camera():ws_server_(), ws_connected_(false){
        // 配置 WebSocket 伺服器
        ws_server_.clear_access_channels(websocketpp::log::alevel::all);
        ws_server_.clear_error_channels(websocketpp::log::elevel::all);
        
        ws_server_.init_asio();
        
        // 設置回調函數
        ws_server_.set_open_handler(bind(&Camera::on_open, this, std::placeholders::_1));
        ws_server_.set_close_handler(bind(&Camera::on_close, this, std::placeholders::_1));

        cb.streamer = this;
        camera.registerCallback(&cb);

        camera.start();
    };

    ~Camera(){
        camera.stop();
        if (ws_thread_.joinable()) {
            ws_thread_.join();
        }
    };

    void updateImage(const cv::Mat &mat){
        std::vector<uchar> jpeg_buf;
        cv::imencode(".jpg", mat, jpeg_buf);
        websocketpp::lib::error_code ec;
        ws_server_.send(ws_hdl_, jpeg_buf.data(), jpeg_buf.size(), websocketpp::frame::opcode::binary,ec);
        if (ec){
            std::cerr << "send failed: " << ec.message() << std::endl;
        }
    }

    // WebSocket 連線處理函數
    void on_open(websocketpp::connection_hdl hdl) {
        std::lock_guard<std::mutex> lock(mutex_);
        ws_hdl_ = hdl;
        ws_connected_ = true;
        std::cout << "WebSocket Client Connected!" << std::endl;
    }

    void on_close(websocketpp::connection_hdl hdl) {
        std::lock_guard<std::mutex> lock(mutex_);
        ws_connected_ = false;
        std::cout << "WebSocket Client Disconnected!" << std::endl;
    }

    // 啟動 WebSocket 伺服器
    void start_websocket_server() {
        try {
            ws_server_.listen(8080);
            ws_server_.start_accept();
            std::cout << "WebSocket server started on port 8080" << std::endl;
            ws_server_.run();
        } catch (const std::exception& e) {
            std::cerr << "WebSocket server error: " << e.what() << std::endl;
        }
    }

    bool start(){
        ws_thread_ = std::thread(&Camera::start_websocket_server, this);
        return true;
    }
    struct libcamera2opencv_Callback : Libcam2OpenCV::Callback{
        Camera *streamer = nullptr;

        virtual void hasFrame(const cv::Mat &frame, const libcamera::ControlList &){
            std::cout << "Frame received, size = " << frame.cols << "x" << frame.rows << std::endl;
            if (nullptr != streamer){
                streamer->updateImage(frame);
            }
        }
    };

private:
    
    Libcam2OpenCV camera;
    libcamera2opencv_Callback cb;

    websocket_server ws_server_;
    websocketpp::connection_hdl ws_hdl_;

    bool ws_connected_;
    std::mutex mutex_;  // 用於保護 WebSocket 連接狀態
    std::thread ws_thread_;

};

#endif