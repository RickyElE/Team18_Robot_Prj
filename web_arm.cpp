// 文件：web_arm.cpp

#include "web_arm.h"

#include <iostream>
#include <unistd.h>         // usleep()
#include <algorithm>        // std::min/max
#include <nlohmann/json.hpp>

#include "Control.h"        // 你的 SCServo 库（Control 类）
#include "RoboArm.h"        // 你第一段代码：UpdateArmState, initial_roboarm, MoveForward..., HeadUp...
#include "PCA9685.h"        // 夹爪

// —————— 全局对象和状态 ——————
static Control   sc;     // SCServo 控制对象
static ArmState  armSt;  // 头部/基座状态缓存

// PCA9685 夹爪通道状态
static PCA9685 pca;
static double  angle_ch2 = 90.0;  // 通道2：手腕
static double  angle_ch3 = 100.0; // 通道3：夹爪

// 角度 → tick
static int angleToTicks(double angle) {
    const int min_pulse = 500, max_pulse = 2400, range_deg = 180;
    double pulse = min_pulse + (max_pulse - min_pulse) * angle / range_deg;
    return static_cast<int>(pulse / 20000.0 * 4096);
}

// —————— 夹爪控制 ——————
inline void RR() {
    angle_ch2 = std::min(180.0, angle_ch2 + 10.0);
    pca.setPWM(2, 0, angleToTicks(angle_ch2));
    std::cout << "[夹爪] 手腕右转 → 通道2 = " << angle_ch2 << "°\n";
}
inline void RL() {
    angle_ch2 = std::max(0.0, angle_ch2 - 10.0);
    pca.setPWM(2, 0, angleToTicks(angle_ch2));
    std::cout << "[夹爪] 手腕左转 → 通道2 = " << angle_ch2 << "°\n";
}
inline void cut() {
    angle_ch3 = std::min(180.0, angle_ch3 + 50.0);
    pca.setPWM(3, 0, angleToTicks(angle_ch3));
    std::cout << "[夹爪] 夹紧 → 通道3 = " << angle_ch3 << "°\n";
}
inline void release() {
    angle_ch3 = std::max(0.0, angle_ch3 - 50.0);
    pca.setPWM(3, 0, angleToTicks(angle_ch3));
    std::cout << "[夹爪] 放松 → 通道3 = " << angle_ch3 << "°\n";
}

// 保证 WebSocket 回调线程安全
static std::mutex ws_mutex;

// —————— WebSocket 控制映射 ——————
inline void MoveUpWS() {
    std::lock_guard<std::mutex> lock(ws_mutex);
    HeadUp(sc, armSt, 30, 1500);
    std::cout << "[反馈] 头部舵机 → " << armSt.headPos << "\n";
}
inline void MoveDownWS() {
    std::lock_guard<std::mutex> lock(ws_mutex);
    HeadDown(sc, armSt, 30, 1500);
    std::cout << "[反馈] 头部舵机 → " << armSt.headPos << "\n";
}
inline void ForwardWS() {
    std::lock_guard<std::mutex> lock(ws_mutex);
    armForward(sc, armSt, 30);
    std::cout << "[反馈] 基座舵机 → " << armSt.basePos << "\n";
}
inline void BackwardWS() {
    std::lock_guard<std::mutex> lock(ws_mutex);
    armBackward(sc, armSt, 30);
    std::cout << "[反馈] 基座舵机 → " << armSt.basePos << "\n";
}

// —————— RoboArmWebSocketServer 实现 ——————
RoboArmWebSocketServer::RoboArmWebSocketServer() : ws_connected_(false) {
    ws_server_.clear_access_channels(websocketpp::log::alevel::all);
    ws_server_.clear_error_channels(websocketpp::log::elevel::all);
    ws_server_.init_asio();
    ws_server_.set_open_handler (
        std::bind(&RoboArmWebSocketServer::on_open,  this, std::placeholders::_1));
    ws_server_.set_close_handler(
        std::bind(&RoboArmWebSocketServer::on_close, this, std::placeholders::_1));
    ws_server_.set_message_handler(
        std::bind(&RoboArmWebSocketServer::on_message,this,
                  std::placeholders::_1, std::placeholders::_2));
}

void RoboArmWebSocketServer::on_open(websocketpp::connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(conn_mutex_);
    ws_connected_ = true; ws_hdl_ = hdl;
    std::cout << "[WebSocket] 客户端已连接\n";
}

void RoboArmWebSocketServer::on_close(websocketpp::connection_hdl) {
    std::lock_guard<std::mutex> lock(conn_mutex_);
    ws_connected_ = false;
    std::cout << "[WebSocket] 客户端已断开\n";
}

void RoboArmWebSocketServer::on_message(websocketpp::connection_hdl, server::message_ptr msg) {
    auto j = nlohmann::json::parse(msg->get_payload());
    if (j.value("type","") != "command") return;
    auto action = j["action"].get<std::string>();
    std::cout << "[WebSocket] 收到命令: " << action << "\n";

    if      (action == "up")        MoveUpWS();
    else if (action == "down")      MoveDownWS();
    else if (action == "For")       ForwardWS();
    else if (action == "Back")      BackwardWS();
    else if (action == "Rotate_R")  RR();
    else if (action == "Rotate_L")  RL();
    else if (action == "cut")       cut();
    else if (action == "release")   release();
    else if (action == "quit")      ws_server_.stop_listening();
}

void RoboArmWebSocketServer::start_server() {
    ws_server_.listen(8084);
    ws_server_.start_accept();
    std::cout << "[WebSocket] 监听端口 8084...\n";
    ws_server_.run();
}

// —————— main ——————
int main() {
    // 1) 初始化 SCServo / Control
    if (!sc.Begin(115200, "/dev/ttyUSB0")) {
        std::cerr << "[错误] SCServo 初始化失败\n";
        return 1;
    }
    // 读状态并同步初始位置
    UpdateArmState(sc, armSt);
    initial_roboarm (sc, armSt);

    // 2) 初始化 PCA9685
    if (!pca.init()) {
        std::cerr << "[错误] PCA9685 初始化失败\n";
        sc.End();
        return 1;
    }
    pca.setPWMFreq(50);
    pca.wakeup();
    pca.setPWM(2,0,angleToTicks(angle_ch2));
    pca.setPWM(3,0,angleToTicks(angle_ch3));

    // 4) 启动 WebSocket
    RoboArmWebSocketServer server;
    server.start_server();

    // 5) 清理
    pca.setPWM(2,0,0);
    
    pca.setPWM(3,0,0);
    sc.End();
    return 0;
}
