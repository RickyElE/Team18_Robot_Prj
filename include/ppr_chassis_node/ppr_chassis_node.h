#ifndef _PPR_CHASSIS_NODE_H_
#define _PPR_CHASSIS_NODE_H_

#include "ChassisPublisher.h"
#include "ChassisSubscriber.h"
#include "bms.h"

#include <thread>
#include <chrono>
#include <functional>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

class ppr_chassis_node
{
public:
    ChassisPublisher* chassisPublisher = new ChassisPublisher();
    ChassisSubscriber* chassisSubscriber = new ChassisSubscriber();

    ppr_chassis_node(){};
    virtual ~ppr_chassis_node(){};
    void run(ppr_chassis_node &mynode);

    bool init();
    void publish_callback(std::function<void(ChassisMsg&)> callback, ChassisMsg chassisMsg);
    void subscribe_callback(std::function<void()> callback);

    
};

bool ppr_chassis_node::init(){
    if (!chassisPublisher->init())
    {
        std::cout << "Failed to initialize publisher" << std::endl;
        return false;
    }

    // if (!batterySubscriber->init())
    // {
    //     std::cout << "Failed to initialize subscriber" << std::endl;
    //     return false;
    // }
    return true;
}

void ppr_chassis_node::publish_callback(std::function<void(ChassisMsg&)> callback, ChassisMsg chassisMsg){
    callback(chassisMsg);
}

void ppr_chassis_node::subscribe_callback(std::function<void()> callback){
    callback();
}

void ppr_chassis_node::run(ppr_chassis_node &mynode){
    std::cout << "ppr_chassis_node is running" << std::endl;
    ChassisMsg chassisMsg;
    BMS bms;
    double voltage = 0;
    double percentage = 0;
    
    while(true)
    {
        voltage = bms.getVoltage();
        percentage = bms.getPercentage();
        chassisMsg.setBatteryVoltage(voltage);
        chassisMsg.setBatteryPercentage(percentage);
        mynode.publish_callback(std::bind(&ChassisPublisher::publish, mynode.chassisPublisher, std::placeholders::_1), chassisMsg);
        // {
        //     std::cout << "No subscribers, battery message not sent" << std::endl;
        // }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        // mynode.subscribe(std::bind(&BatterySubscriber::subscribe, mynode.batterySubscriber));
        // std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
#endif