#ifndef WEBSOCKET_ROBOARM_H
#define WEBSOCKET_ROBOARM_H

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>
#include <mutex>

// 定义 websocketpp 服务器类型
typedef websocketpp::server<websocketpp::config::asio> server;

class RoboArmWebSocketServer {
public:
    RoboArmWebSocketServer();
    
    // 启动服务器（阻塞调用）
    void start_server();
    
    // WebSocket 回调函数
    void on_open(websocketpp::connection_hdl hdl);
    void on_close(websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl hdl, server::message_ptr msg);

private:
    server ws_server_;
    websocketpp::connection_hdl ws_hdl_;
    bool ws_connected_;
    std::mutex conn_mutex_;
};

// 用于机械臂按键控制的函数声明
void MoveUp();
void MoveDown();

#endif // WEBSOCKET_ROBOARM_H
