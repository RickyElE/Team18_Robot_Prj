#include "web_arm.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <unistd.h>       // usleep()
#include "SCServo.h"
#include "UserInput.h"
#include "RoboArm.h"
static int targetPosArm;
// 全局 SCServo 实例（请确保你的 SCServo 库正常引用）
SCSCL sc;

// 舵机编号，根据实际连接情况设置
const int Servo_ID  = 0; //for/back
const int Servo_ID1 = 1; //up/down
int targetPos;//updown
int targetpos1;//forback

// 用于保证舵机操作线程安全的互斥量
std::mutex servo_mutex;

//--------------------------------------------------
// 机械臂控制函数：MoveUp() 和 MoveDown()
//--------------------------------------------------
inline void MoveUp() {
    std::lock_guard<std::mutex> lock(servo_mutex);
    // 采用一个 static 变量记录 ID1 的目标位置
    // 初始化时从舵机反馈中读取当前的位置
    // static int targetPos = sc.ReadPos(Servo_ID1);
    
    targetPos += 50;  // 每次加 50
    std::cout << "[控制] 执行 MoveUp(), 新目标位置: " << targetPos << std::endl;
    
    // 将新的目标位置写入 ID1 舵机，参数：(u16)目标位置，0 为模式参数，1500 为运动速度
    sc.WritePos(Servo_ID1, (u16)targetPos, 0, 1500);
    // 延时确保舵机有足够时间运动到目标位置
    usleep(754 * 1000);

    if (sc.FeedBack(Servo_ID1) != -1) {
        int realPos = sc.ReadPos(Servo_ID1);
        std::cout << "[反馈] ID1实际位置: " << realPos << std::endl;
    } else {
        std::cerr << "[反馈] 读取舵机反馈失败" << std::endl;
    }
}

inline void MoveDown() {
    std::lock_guard<std::mutex> lock(servo_mutex);
    // 同样使用相同的 static 变量以保持当前位置
    // static int targetPos = sc.ReadPos(Servo_ID1);
    
    targetPos -= 50;  // 每次减 50
    std::cout << "[控制] 执行 MoveDown(), 新目标位置: " << targetPos << std::endl;
    
    sc.WritePos(Servo_ID1, (u16)targetPos, 0, 1500);
    usleep(754 * 1000);

    if (sc.FeedBack(Servo_ID1) != -1) {
        int realPos = sc.ReadPos(Servo_ID1);
        std::cout << "[反馈] ID1实际位置: " << realPos << std::endl;
    } else {
        std::cerr << "[反馈] 读取舵机反馈失败" << std::endl;
    }
}

inline void Forward() {
    std::lock_guard<std::mutex> lock(servo_mutex);
    // 同样使用相同的 static 变量以保持当前位置
    // static int targetPos1 = sc.ReadPos(Servo_ID);
    
    targetPos1 += 50;  // 每次加 50
    std::cout << "[控制] 执行 Forward(), 新目标位置: " << targetPos1 << std::endl;
    
    sc.WritePos(Servo_ID, (u16)targetPos1, 0, 1500);
    usleep(754 * 1000);

    if (sc.FeedBack(Servo_ID) != -1) {
        int realPos = sc.ReadPos(Servo_ID);
        std::cout << "[反馈] ID0实际位置: " << realPos << std::endl;
    } else {
        std::cerr << "[反馈] 读取舵机反馈失败" << std::endl;
    }
}

inline void Backward() {
    std::lock_guard<std::mutex> lock(servo_mutex);
    // 同样使用相同的 static 变量以保持当前位置
    // static int targetPos = sc.ReadPos(Servo_ID);
    
    targetPos1 -= 50;  // 每次减 50
    std::cout << "[控制] 执行 Backward(), 新目标位置: " << targetPos1 << std::endl;
    
    sc.WritePos(Servo_ID, (u16)targetPos1, 0, 1500);
    usleep(754 * 1000);

    if (sc.FeedBack(Servo_ID) != -1) {
        int realPos = sc.ReadPos(Servo_ID);
        std::cout << "[反馈] ID0实际位置: " << realPos << std::endl;
    } else {
        std::cerr << "[反馈] 读取舵机反馈失败" << std::endl;
    }
}

//--------------------------------------------------
// RoboArmWebSocketServer 类成员函数实现
//--------------------------------------------------
RoboArmWebSocketServer::RoboArmWebSocketServer() : ws_connected_(false) {
    // 关闭默认日志，便于调试时观察控制台输出
    ws_server_.clear_access_channels(websocketpp::log::alevel::all);
    ws_server_.clear_error_channels(websocketpp::log::alevel::all);
    ws_server_.init_asio();

    // 设置各个事件回调函数
    ws_server_.set_open_handler(std::bind(&RoboArmWebSocketServer::on_open, this, std::placeholders::_1));
    ws_server_.set_close_handler(std::bind(&RoboArmWebSocketServer::on_close, this, std::placeholders::_1));
    ws_server_.set_message_handler(std::bind(&RoboArmWebSocketServer::on_message, this,
                                               std::placeholders::_1, std::placeholders::_2));
}

void RoboArmWebSocketServer::on_open(websocketpp::connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(conn_mutex_);
    ws_hdl_ = hdl;
    ws_connected_ = true;
    std::cout << "[WebSocket] 客户端连接成功。" << std::endl;
}

void RoboArmWebSocketServer::on_close(websocketpp::connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(conn_mutex_);
    ws_connected_ = false;
    std::cout << "[WebSocket] 客户端断开连接。" << std::endl;
}

void RoboArmWebSocketServer::on_message(websocketpp::connection_hdl hdl, server::message_ptr msg) {
    try {
        std::string payload = msg->get_payload();
        auto json_msg = nlohmann::json::parse(payload);

        if (json_msg.contains("type") && json_msg["type"] == "command") {
            std::string action = json_msg["action"];
            std::cout << "[WebSocket] 收到控制命令: " << action << std::endl;

            // 根据接收到的命令调用对应的机械臂控制函数
            if (action == "up") {
                std::cout << "up!" << std::endl;
                MoveUp();
            } else if (action == "down") {
                std::cout << "down!" << std::endl;
                MoveDown();
            } else if (action == "For") {
                std::cout << "Forward!" << std::endl;
                Forward();
            } else if (action == "Back") {
                std::cout << "Backward!" << std::endl;
                Backward();
            // } else if (action == "Rotate_R") {
            //     std::cout << "Rotate_R!" << std::endl;
            //     Rotate_R();
            // } else if (action == "Rotate_L") {
            //     std::cout << "Rotate_L!" << std::endl;
            //     Rotate_L();
            // } else if (action == "cut") {
            //     std::cout << "cut!" << std::endl;
            //     cut();
            // } else if (action == "release") {
            //     std::cout << "release!" << std::endl;
            //     release();
            } else if (action == "quit") {
                std::cout << "[WebSocket] 收到退出命令，准备关闭服务器..." << std::endl;
                ws_server_.stop_listening();
            }
        }
    }
    catch (std::exception& e) {
        std::cerr << "[WebSocket] 消息处理错误: " << e.what() << std::endl;
    }
}

void RoboArmWebSocketServer::start_server() {
    try {
        ws_server_.listen(8084);
        ws_server_.start_accept();
        std::cout << "[WebSocket] 服务已在端口 8084 启动。" << std::endl;
        ws_server_.run();
    }
    catch (const std::exception& e) {
        std::cerr << "[WebSocket] 服务器错误: " << e.what() << std::endl;
    }
}

//--------------------------------------------------
// 主函数：初始化舵机及机械臂，再启动 WebSocket 服务
//--------------------------------------------------
int main(int argc, char **argv) {
    // 初始化 SCServo（默认串口 "/dev/ttyUSB0"，波特率 115200）
    if (!sc.begin(115200, "/dev/ttyUSB0")) {
        std::cerr << "[错误] 初始化 SCServo 失败！" << std::endl;
        return -1;
    }

    // 机械臂初始定位（该函数定义在 RoboArm.h 中，请根据实际情况调整初始位置）
    // initial_roboarm(sc);
    // 延时确保舵机完成初始运动
//     usleep(754 * 1000);
//       sc.FeedBack(Servo_ID1);
//   targetPosArm = sc.ReadPos(Servo_ID1);
    // 先读两路舵机当前位置，初始化共享变量
    targetPos  = sc.ReadPos(Servo_ID1);
    targetPos1 = sc.ReadPos(Servo_ID);
    // 读取并显示反馈信息（这里只读取 ID0 和 ID1 的位置，可自行调整）
    int feedbackPos = sc.ReadPos(Servo_ID);
    int feedbackPos1 = sc.ReadPos(Servo_ID1);
    std::cout << "[信息] 初始位置反馈：" << std::endl;
    std::cout << "    舵机 " << Servo_ID << " 的反馈位置 = " << feedbackPos << std::endl;
    std::cout << "    舵机 " << Servo_ID1 << " 的反馈位置 = " << feedbackPos1 << std::endl;

    std::cout << "[提示] 网页端控制说明：" << std::endl;
    std::cout << "    发送 JSON 命令 { \"type\": \"command\", \"action\": \"move_up\" }    可向上移动机械臂 (ID0 +50)" << std::endl;
    std::cout << "    发送 JSON 命令 { \"type\": \"command\", \"action\": \"move_down\" }  可向下移动机械臂 (ID0 -50)" << std::endl;
    std::cout << "    发送 JSON 命令 { \"type\": \"command\", \"action\": \"quit\" }       可退出程序" << std::endl;

    // 创建并启动 WebSocket 服务器线程
    RoboArmWebSocketServer ws_server;
    std::thread ws_thread([&ws_server]() {
        ws_server.start_server();
    });

    // 等待 WebSocket 服务器线程退出
    ws_thread.join();

    // 程序退出前关闭 SCServo 连接
    sc.end();
    std::cout << "[结束] 程序退出。" << std::endl;
    return 0;
}