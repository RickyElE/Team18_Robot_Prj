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
        motor.Wakeup();
        bms.start();

        cpuTemp_callback.systeminfo = &cpuTempture;

        cpuTempture.Init(systemInfo::CPU_TEMP);

        cpuTempture.registerCallback(&cpuTemp_callback);

        cpuTempture.Start();


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
            }
        }
        catch(std::exception& e){
            std::cerr << "Error handling message: " << e.what() << std::endl;
        }
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

    // 發送電池數據到 WebSocket
    void send_battery_status() {
        while(running_){
            std::lock_guard<std::mutex> lock(mutex_);
            float voltage = bms.getVoltage();
            float percentage = bms.getPercentage();
            double temperature = cpuTempture.getCPUTempture();
            if (ws_connected_) {
                try {
                    nlohmann::json msg = {
                        {"voltage", voltage}, 
                        {"percentage", percentage}, 
                        {"cpu", {
                            "temperature",temperature
                        }}
                    };
                    ws_server_.send(ws_hdl_, msg.dump(), websocketpp::frame::opcode::text);
                    std::cout << "Sent battery data to WebSocket: " << msg.dump() << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "Error sending to WebSocket: " << e.what() << std::endl;
                }
            }
        }
        
    }

    // void send_SystemInfo(){
    //     while(sysInfo_running_){
    //         double temperature = cpuTempture.getCPUTempture();
    //         if (ws_connected_){
    //             try{
    //                 nlohmann::json msg = {{"cpu-temperature", temperature}};
    //                 ws_server_.send(ws_hdl_, msg.dump(), websocketpp::frame::opcode::text);
    //                 std::cout << "Sent cpu-temperature to WebSocket: " << msg.dump() << std::endl;
    //             }catch (const std::exception& e) {
    //                 std::cerr << "Error sending to WebSocket: " << e.what() << std::endl;
    //             }
    //         }
    //     }
    // }

    bool start(){
        running_ = true;
        sysInfo_running_ = true;
        bms_thread_ = std::thread(&WebsiteServer::send_battery_status,this);
        // 啟動 WebSocket 伺服器線程
        ws_thread_ = std::thread(&WebsiteServer::start_websocket_server, this);

        // sysinfo_thread_ = std::thread(&WebsiteServer::send_SystemInfo, this);
        return true;
    }

    void stop(){
        running_ = false;
        sysInfo_running_ = false;
        cpuTempture.Stop();
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
private:
    std::atomic<bool> running_ = false;
    std::atomic_bool sysInfo_running_ = false;
    std::thread ws_thread_;
    std::thread bms_thread_;
    std::thread sysinfo_thread_;

    websocket_server ws_server_;
    websocketpp::connection_hdl ws_hdl_;
    bool ws_connected_;
    std::mutex mutex_;

    SystemInfo cpuTempture;
    SystemInfo cpuUsage;

    Mycallback cpuTemp_callback;
    Mycallback cpuUsage_callback;

    Motor motor = Motor();
    Delay delay = Delay();
    BMS   bms;
    

};

#endif