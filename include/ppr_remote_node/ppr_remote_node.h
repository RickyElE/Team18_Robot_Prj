#ifndef __PPR_REMOTE_NODE_H__
#define __PPR_REMOTE_NODE_H__

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp> // JSON 庫
#include "Subscriber.h"
#include "BatteryMsg.h"
#include "BatteryMsgPubSubTypes.h"

#include <atomic>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>
using namespace websocketpp;
using json = nlohmann::json;

typedef server<config::asio> websocket_server;
websocket_server ws_server;
connection_hdl ws_hdl;
bool ws_connected = false;


class ppr_remote_node{
public:
    ppr_remote_node(){};
    ~ppr_remote_node(){
        if (worker_.joinable())
        {
            worker_.join();
        }
    };

    void run();
    bool start(){
        running_ = true;
        worker_ = std::thread(&ppr_remote_node::run, this);
        return true;
    }
private:
    std::atomic<bool> running_ = false;
    std::thread worker_;
};

// WebSocket 連線處理函數
void on_open(connection_hdl hdl) {
    ws_hdl = hdl;
    ws_connected = true;
    std::cout << "WebSocket Client Connected!" << std::endl;
}

void on_close(connection_hdl hdl) {
    ws_connected = false;
    std::cout << "WebSocket Client Disconnected!" << std::endl;
}

// 啟動 WebSocket 伺服器
void start_websocket_server() {
    ws_server.init_asio();
    ws_server.set_open_handler(&on_open);
    ws_server.set_close_handler(&on_close);
    ws_server.listen(9002);
    ws_server.start_accept();
    ws_server.run();
}

// 發送電池數據到 WebSocket
void send_battery_status(float voltage, float percentage) {
    if (ws_connected) {
        json msg = {{"voltage", voltage}, {"percentage", percentage}};
        ws_server.send(ws_hdl, msg.dump(), frame::opcode::text);
    }
}

void ppr_remote_node::run(){
    std::cout << "Running ppr_remote_node" << std::endl;
    BatteryMsg battery_msg{};
    eprosima::fastdds::dds::DomainParticipantQos pqos;
    pqos.name("Participant_Subscriber");
    Subscriber<BatteryMsgPubSubType, BatteryMsg> battery_subscriber(BatteryMsgPubSubType(),&battery_msg,
    "Battery_Topic", eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(0,pqos));
    battery_subscriber.init();
    while(running_){
        // BatteryMsg msg;
        battery_subscriber.listener_->wait_for_data();
        // msg = battery_subscriber.listener_->get_data();
        // std::cout<<"Voltage is: "<<msg.voltage() << " ,Percentage is : "<<msg.percentage()<<std::endl;
        std::cout<<"battery_msg: Voltage is: "<<battery_msg.voltage() << " ,Percentage is : "<<battery_msg.percentage()<<std::endl;
    }
}
#endif