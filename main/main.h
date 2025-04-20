#ifndef __MAIN_H__
#define __MAIN_H__

#include <websocketpp/common/connection_hdl.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/logger/levels.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp> // JSON 庫

#include <atomic>
#include <thread>
#include <chrono>
#include <mutex>
#include <functional>

#include "motor.h"
#include "delay.h"
#include "bms.h"
#include "SystemInfo.h"
#include "Ultrasonic.h"

#include "libcam2web.h"
#include "libcam2opencv.h"
#include "simple_http_server.h"

#include "web_arm.h"

// #include "../include/camera_server/bridge.cpp"

typedef websocketpp::server<websocketpp::config::asio> websocket_server;

class WebsiteServer{
public:
    WebsiteServer() : ws_server_(), ws_connected_(false){
        ws_server_.clear_access_channels(websocketpp::log::alevel::all);
        ws_server_.clear_error_channels(websocketpp::log::alevel::all);

        ws_server_.init_asio();

        ws_server_.set_open_handler(bind(&WebsiteServer::on_open, this, std::placeholders::_1));
        ws_server_.set_close_handler(bind(&WebsiteServer::on_close, this, std::placeholders::_1));
        ws_server_.set_message_handler(bind(&WebsiteServer::on_message, this, std::placeholders::_1, std::placeholders::_2));

        if(!motor.init()){
            throw "Motor cannot initialize!\n";
        }
        if (!ultrasonic.init()){
            throw "Ultrasonic cannot initialize!\n";
        }
 
        {
            ultrasonic.registerCallback(&dist);
            ultrasonic.start();
        }
        motor.Wakeup();

        if(!arm.Initial_Arm()){
            throw "Arm cannot initialize!\n";
        }
        bms.start();

        // CPU Temperature Initialize
        cpuTemp_callback.systeminfo = &cpuTempture;

        cpuTempture.Init(systemInfo::CPU_TEMP);

        cpuTempture.registerCallback(&cpuTemp_callback);

        cpuTempture.Start();
        
        // CPU Usage Initialize
        cpuUsage_callback.systeminfo = &cpuUsage;

        cpuUsage.Init(systemInfo::CPU_USAGE);

        cpuUsage.registerCallback(&cpuUsage_callback);

        cpuUsage.Start();

        // Memory Usage Initialize
        memUsage_callback.systeminfo = &memUsage;

        memUsage.Init(systemInfo::RAM_USAGE);

        memUsage.registerCallback(&memUsage_callback);

        memUsage.Start();

        // Swap Usage Initialize
        swapUsage_callback.systeminfo = &swapUsage;

        swapUsage.Init(systemInfo::SWAP_USAGE);

        swapUsage.registerCallback(&swapUsage_callback);

        swapUsage.Start();

        cameracallback.server = this;
        camera.registerCallback(&cameracallback);
        camera_settings_.width = 1280;
        camera_settings_.height = 720;
        camera_settings_.framerate = 30;
        camera_settings_.brightness = 0.0f;
        camera_settings_.contrast = 1.0f;
        camera.start(camera_settings_);
    }

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

    void on_message(websocketpp::connection_hdl hdl, websocket_server::message_ptr msg){
        try{
            std::string payload = msg->get_payload();
            auto json_msg = nlohmann::json::parse(payload);

            if (json_msg.contains("type") && json_msg["type"] == "command"){
                std::string action = json_msg["action"];
                std::cout << "Received command from web: " << action << std::endl;

                if (action == "move_forward"){
                    std::cout << "Move_Forward!" << std::endl;
                    motor.Forward();
                }
                else
                if(action == "stop"){
                    std::cout << "Stop!" << std::endl;
                    motor.Stop();
                }
                else
                if(action == "move_back"){
                    std::cout << "Backward!" << std::endl;
                    motor.Backward();
                }
                else
                if(action == "move_left"){
                    std::cout << "TurnLeft!" << std::endl;
                    motor.TurnLeft();
                }
                else
                if(action == "move_right"){
                    std::cout << "TurnRight!" << std::endl;
                    motor.TurnRight();
                }
                else if(action == "start_camera"){
                    std::cout << "starting camera" << std::endl;
                }
                else if(action == "stop_camera"){
                    std::cout << "stopping camera..." << std::endl;
                }
                else if(action == "take_screenshot"){
                    std::cout << "接收到截圖請求！" << std::endl;
                    
                    sendScreenshotResponse(hdl);
                }
                else if(action == "camera_move_up"){
                    std::cout << "Camera UP!" << std::endl;
                    motor.CameraUp();
                    
                }
                else if(action == "camera_move_down"){
                    std::cout << "Camera DOWN!" << std::endl;
                    motor.CameraDown();
                }
                else if(action == "camera_move_stop"){
                    std::cout << "Camera cleanServoAngle!" << std::endl;
                    motor.cleanServoAngle();
                }
                else if(action == "up"){
                    std::cout << "Head UP!" << std::endl;
                    arm.MoveUpWS();
                }
                else if(action == "down"){
                    std::cout << "Head DOWN!" << std::endl;
                    arm.MoveDownWS();
                }
                else if(action == "For"){
                    std::cout << "Arm FORWARD!" << std::endl;
                    arm.ForwardWS();
                }
                else if(action == "Back"){
                    std::cout << "Arm BACKWARD!" << std::endl;
                    arm.BackwardWS();
                }
                else if(action == "Rotate_R"){
                    std::cout << "Rotate RIGHT!" << std::endl;
                    arm.RR();
                }
                else if(action == "Rotate_L"){
                    std::cout << "Rotate LEFT!" << std::endl;
                    arm.RL();
                }
                else if(action == "cut"){
                    std::cout << "CUT!" << std::endl;
                    arm.cut();
                }
                else if(action == "release"){
                    std::cout << "RELEASE!" << std::endl;
                    arm.release();
                }
            }
        }
        catch(std::exception& e){
            std::cerr << "Error handling message: " << e.what() << std::endl;
        }
    }

    void sendScreenshotResponse(websocketpp::connection_hdl hdl) {
        
        std::cout << "截圖功能尚未實現" << std::endl;
    }

    void start_websocket_server() {
        try {
            ws_server_.listen(8084);
            ws_server_.start_accept();
            std::cout << "WebSocket server started on port 8084" << std::endl;
            ws_server_.run();
        } catch (const std::exception& e) {
            std::cerr << "WebSocket server error: " << e.what() << std::endl;
        }
    }


    void send_battery_status() {
        while(running_){
            std::lock_guard<std::mutex> lock(mutex_);
            float voltage = bms.getVoltage();
            float percentage = bms.getPercentage();
            double temperature = cpuTempture.getCPUTempture();
            double usage = cpuUsage.getCpuUsage();
            double mem_usage = memUsage.getMemUsage();
            double swap_usage = swapUsage.getSwapUsage();
            double distance = ultrasonic.getDistance();
            if (ws_connected_) {
                try { 
                    msg["voltage"] = voltage;
                    msg["percentage"] = percentage;
                    msg["cpu"]["temperature"] = temperature;
                    msg["cpu"]["usage"] = usage;
                    msg["memory"]["ram"] = mem_usage;
                    msg["memory"]["swap"] = swap_usage;
                    msg["distance"] = distance;
                    ws_server_.send(ws_hdl_, msg.dump(), websocketpp::frame::opcode::text);
                    std::cout << "Sent battery data to WebSocket: " << msg.dump() << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "Error sending to WebSocket: " << e.what() << std::endl;
                }
            }
            

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    bool start(){
        http_server_.setImageProvider([this](){
            return this->getJpegData();
        });
        http_server_.start();
        running_ = true;
        sysInfo_running_ = true;
        bms_thread_ = std::thread(&WebsiteServer::send_battery_status, this);
        

        ws_thread_ = std::thread(&WebsiteServer::start_websocket_server, this);

        return true;
    }

    void stop(){
        running_ = false;
        sysInfo_running_ = false;
        motor.cleanServoAngle();
        camera.stop();
        motor.Stop();
        arm.stop();
        cpuTempture.Stop();
        cpuUsage.Stop();
        memUsage.Stop();
        swapUsage.Stop();
        ultrasonic.stop();
        if(bms_thread_.joinable()){
            bms_thread_.join();
        }
        if(sysinfo_thread_.joinable()){
            sysinfo_thread_.join();
        }
        if(ws_server_.is_listening()){
            ws_server_.stop_listening();
        }
        ws_server_.stop();
        if(ws_thread_.joinable()){
            ws_thread_.join();
        }
    }
    
    void updateFrame(const cv::Mat &frame) {
        std::lock_guard<std::mutex> lock(frameMutex);
        
        // 將幀轉換為JPEG格式
        std::vector<uchar> buf;
        cv::imencode(".jpg", frame, buf, {cv::IMWRITE_JPEG_QUALITY, 90});
        
        // 更新最新的幀數據
        latestFrame.assign(buf.begin(), buf.end());
    }

    struct CameraCallback : Libcam2OpenCV::Callback {
        WebsiteServer* server = nullptr;
        
        virtual void hasFrame(const cv::Mat &frame, const libcamera::ControlList &) {
            if (server != nullptr) {
                server->updateFrame(frame);
            }
        }
    };

    
private:
    std::atomic<bool> running_ = false;
    std::atomic_bool sysInfo_running_ = false;
    std::thread ws_thread_;
    std::thread bms_thread_;
    std::thread sysinfo_thread_;
    std::thread camera_thread_;

    websocket_server ws_server_;
    websocketpp::connection_hdl ws_hdl_;
    bool ws_connected_;
    std::mutex mutex_;

    SystemInfo cpuTempture;
    SystemInfo cpuUsage;
    SystemInfo memUsage;
    SystemInfo swapUsage;

    Mycallback cpuTemp_callback;
    Mycallback cpuUsage_callback;
    Mycallback memUsage_callback;
    Mycallback swapUsage_callback;

    Motor motor = Motor();
    Delay delay = Delay();
    Arm arm;
    BMS bms;
    Ultrasonic ultrasonic;
    DistanceCalculator dist;

    nlohmann::json msg;
    
    // 攝像頭相關成員
    Libcam2OpenCV camera;
    Libcam2OpenCVSettings camera_settings_;
    // 幀存儲
    std::mutex frameMutex;
    std::vector<uint8_t> latestFrame;

    CameraCallback cameracallback;

    std::vector<uint8_t> getJpegData() {
        std::lock_guard<std::mutex> lock(frameMutex);
        return latestFrame;
    }

    SimpleHttpServer http_server_;
};

#endif