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

#include <sys/timerfd.h>

class ppr_chassis_node
{
public:
    ChassisPublisher* chassisPublisher = new ChassisPublisher();
    ChassisSubscriber* chassisSubscriber = new ChassisSubscriber();

    ppr_chassis_node(){};
    virtual ~ppr_chassis_node(){};
    void run();

    bool init();
    void publish_callback(std::function<void(ChassisMsg&)> callback, ChassisMsg chassisMsg);
    void subscribe_callback(std::function<void()> callback);
    bool start(int interval_ms = 1000){
        // if (timer_fd_ != -1){
        //     std::cerr << "Timer already running!" << std::endl;
        //     return false;
        // }

        // timer_fd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
        // if (timer_fd_ == -1) {
        //     std::cerr << "Failed to create timerfd!" << std::endl;
        //     return false;
        // }

        // struct itimerspec new_value {};
        // new_value.it_value.tv_sec = interval_ms / 1000;
        // new_value.it_value.tv_nsec = (interval_ms % 1000) * 1000000;
        // new_value.it_interval.tv_sec = new_value.it_value.tv_sec;
        // new_value.it_interval.tv_nsec = new_value.it_value.tv_nsec;

        // if (timerfd_settime(timer_fd_, 0, &new_value, nullptr) == -1) {
        //     std::cerr << "Failed to set timerfd!" << std::endl;
        //     close(timer_fd_);
        //     timer_fd_ = -1;
        //     return false;
        // }
        running_ = true;
        bms.start();
        worker_ = std::thread(&ppr_chassis_node::run, this);
        return true;
    }
    void stop(){
        running_ = false;
        bms.stop();
        // if (worker_.joinable()){
        //     worker_.join();
        // }
        // if (timer_fd_ != -1){
        //     close(timer_fd_);
        //     timer_fd_ = -1;
        // }
    }
private:
    int timer_fd_;
    std::atomic<bool> running_ = false;
    std::thread worker_;
    BMS bms;
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

void ppr_chassis_node::run(){
    std::cout << "ppr_chassis_node is running" << std::endl;
    ChassisMsg chassisMsg;
    double voltage = 0;
    double percentage = 0;
    // uint64_t expirations;
    while(running_)
    {
        voltage = bms.getVoltage();
        percentage = bms.getPercentage();
        chassisMsg.setBatteryVoltage(voltage);
        chassisMsg.setBatteryPercentage(percentage);
        this->publish_callback(std::bind(&ChassisPublisher::publish, this->chassisPublisher, std::placeholders::_1), chassisMsg);
    }
}
#endif