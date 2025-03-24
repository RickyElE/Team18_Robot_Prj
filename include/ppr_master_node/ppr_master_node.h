#ifndef _PPR_MASTER_NODE_H_
#define _PPR_MASTER_NODE_H_


// #include "ChassisSubscriber.h"
#include "Subscriber.h"
#include "BatteryMsg.h"
#include "BatteryMsgPubSubTypes.h"

#include <atomic>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>

class ppr_master_node
{
public:
    // ChassisSubscriber* chassisSubscriber = new ChassisSubscriber();

    ppr_master_node(){};
    ~ppr_master_node(){
        if (worker_.joinable())
        {
            worker_.join();
        }
    };
    void run();

    bool init();
    bool subscribe(std::function<void()> callback); // 訂閱 Fast DDS 訊息

    bool start(){
        running_ = true;
        worker_ = std::thread(&ppr_master_node::run, this);
        return true;
    }

private:
    std::atomic<bool> running_ = false;
    std::thread worker_;

};

bool ppr_master_node::init(){
    // std::cout << "Initializing ppr_master_node" << std::endl;
    // if (!chassisSubscriber->init())
    // {
    //     std::cout << "Failed to initialize subscriber" << std::endl;
    //     return false;
    // }s
    return true;
}

bool ppr_master_node::subscribe(std::function<void()> callback){
    callback();
}

void ppr_master_node::run(){
    std::cout << "Running ppr_master_node" << std::endl;
    BatteryMsg battery_msg{}; // 用來儲存接收的電池訊息
    eprosima::fastdds::dds::DomainParticipantQos pqos;
    pqos.name("Participant_Subscriber");
    Subscriber<BatteryMsgPubSubType, BatteryMsg> battery_subscriber(BatteryMsgPubSubType(),&battery_msg,
    "Battery_Topic", eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(0,pqos));
    battery_subscriber.init();
    while(running_){
        // BatteryMsg msg;
        battery_subscriber.listener_->wait_for_data(); // 等待新數據
        // msg = battery_subscriber.listener_->get_data();
        // std::cout<<"Voltage is: "<<msg.voltage() << " ,Percentage is : "<<msg.percentage()<<std::endl;
        std::cout<<"battery_msg: Voltage is: "<<battery_msg.voltage() << " ,Percentage is : "<<battery_msg.percentage()<<std::endl;
    }
    // nlohmann::json result;
    // while(true){
    //     if (this->subscribe(std::bind(&ChassisSubscriber::subscribe, this->chassisSubscriber)))
    //     {
    //         result = chassisSubscriber->getJson();
    //     }
    // }
}
#endif