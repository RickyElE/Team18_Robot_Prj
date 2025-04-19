// 文件：web_arm.cpp

#include "web_arm.h"

#include <iostream>
#include <unistd.h>         // usleep()
#include <algorithm>        // std::min/max
#include <nlohmann/json.hpp>

#include "SCServo.h"
#include "PCA9685.h"

// —————— SCServo (机械臂) 部分 ——————
static SCSCL sc;
const int Joint_ForBack = 0;    // 通道 0: 机械臂前后
const int Joint_UpDown  = 1;    // 通道 1: 机械臂上下
std::mutex servo_mutex;

inline void MoveUp() {
    std::lock_guard<std::mutex> lock(servo_mutex);
    static int pos = sc.ReadPos(Joint_UpDown);
    pos = std::min(1023, pos + 50);
    sc.WritePos(Joint_UpDown, (u16)pos, 0, 1500);
    usleep(754000);
    sc.FeedBack(Joint_UpDown);
    std::cout << "[反馈] 上下舵机 " << Joint_UpDown
              << " 实际位置 = " << sc.ReadPos(Joint_UpDown) << "\n";
}

inline void MoveDown() {
    std::lock_guard<std::mutex> lock(servo_mutex);
    static int pos = sc.ReadPos(Joint_UpDown);
    pos = std::max(0, pos - 50);
    sc.WritePos(Joint_UpDown, (u16)pos, 0, 1500);
    usleep(754000);
    sc.FeedBack(Joint_UpDown);
    std::cout << "[反馈] 上下舵机 " << Joint_UpDown
              << " 实际位置 = " << sc.ReadPos(Joint_UpDown) << "\n";
}

inline void Forward() {
    std::lock_guard<std::mutex> lock(servo_mutex);
    static int pos = sc.ReadPos(Joint_ForBack);
    pos = std::min(1023, pos + 50);
    sc.WritePos(Joint_ForBack, (u16)pos, 0, 1500);
    usleep(754000);
    sc.FeedBack(Joint_ForBack);
    std::cout << "[反馈] 前后舵机 " << Joint_ForBack
              << " 实际位置 = " << sc.ReadPos(Joint_ForBack) << "\n";
}

inline void Backward() {
    std::lock_guard<std::mutex> lock(servo_mutex);
    static int pos = sc.ReadPos(Joint_ForBack);
    pos = std::max(0, pos - 50);
    sc.WritePos(Joint_ForBack, (u16)pos, 0, 1500);
    usleep(754000);
    sc.FeedBack(Joint_ForBack);
    std::cout << "[反馈] 前后舵机 " << Joint_ForBack
              << " 实际位置 = " << sc.ReadPos(Joint_ForBack) << "\n";
}

// —————— PCA9685 (夹爪) 部分 ——————
static PCA9685 pca;
static double angle_ch2 = 90.0;  // 通道2：手腕
static double angle_ch3 = 100.0; // 通道3：夹爪

// 角度(0~180) → tick
static int angleToTicks(double angle) {
    const int min_pulse   = 500;   // μs
    const int max_pulse   = 2400;  // μs
    const int range_deg   = 180;   // 伺服实际范围
    double pulse = min_pulse + (max_pulse - min_pulse) * angle / range_deg;
    return static_cast<int>(pulse / 20000.0 * 4096);
}

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

// —————— WebSocket 服务 ——————
RoboArmWebSocketServer::RoboArmWebSocketServer() : ws_connected_(false) {
    ws_server_.clear_access_channels(websocketpp::log::alevel::all);
    ws_server_.clear_error_channels(websocketpp::log::elevel::all);
    ws_server_.init_asio();
    ws_server_.set_open_handler(
        std::bind(&RoboArmWebSocketServer::on_open, this, std::placeholders::_1));
    ws_server_.set_close_handler(
        std::bind(&RoboArmWebSocketServer::on_close, this, std::placeholders::_1));
    ws_server_.set_message_handler(
        std::bind(&RoboArmWebSocketServer::on_message, this,
                  std::placeholders::_1, std::placeholders::_2));
}

void RoboArmWebSocketServer::on_open(websocketpp::connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(conn_mutex_);
    ws_connected_ = true;
    ws_hdl_ = hdl;
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

    if      (action == "up")       MoveUp();
    else if (action == "down")     MoveDown();
    else if (action == "For")      Forward();
    else if (action == "Back")     Backward();
    else if (action == "Rotate_R") RR();
    else if (action == "Rotate_L") RL();
    else if (action == "cut")      cut();
    else if (action == "release")  release();
    else if (action == "quit")     ws_server_.stop_listening();
}

void RoboArmWebSocketServer::start_server() {
    ws_server_.listen(8084);
    ws_server_.start_accept();
    std::cout << "[WebSocket] 监听端口 8084...\n";
    ws_server_.run();
}

// —————— main ——————
int main() {
    // 1) 初始化 SCServo
    if (!sc.begin(115200, "/dev/ttyUSB0")) {
        std::cerr << "[错误] SCServo 初始化失败\n";
        return 1;
    }

    // 2) 初始化 PCA9685
    if (!pca.init()) {
        std::cerr << "[错误] PCA9685 初始化失败\n";
        return 1;
    }
    pca.setPWMFreq(50);
    // === 关键：唤醒 PCA9685，清除 SLEEP 位 ===
    pca.wakeup();

    // 将夹爪通道归位
    pca.setPWM(2, 0, angleToTicks(angle_ch2));
    pca.setPWM(3, 0, angleToTicks(angle_ch3));

    // 3) 打印控制说明
    std::cout
        << "Web 控制说明：\n"
        << "  up        → 机械臂 上\n"
        << "  down      → 机械臂 下\n"
        << "  For       → 机械臂 前\n"
        << "  Back      → 机械臂 后\n"
        << "  Rotate_R  → 手腕 右转\n"
        << "  Rotate_L  → 手腕 左转\n"
        << "  cut       → 夹爪 关紧\n"
        << "  release   → 夹爪 放松\n"
        << "  quit      → 退出服务\n\n";

    // 4) 启动 WebSocket 服务器（阻塞）
    RoboArmWebSocketServer server;
    server.start_server();

    // 5) 清理 SCServo
    sc.end();
    return 0;
}
