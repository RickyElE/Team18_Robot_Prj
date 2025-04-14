#include "SystemInfo.h"
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <thread>

bool SystemInfo::Init(systemInfo type){
    if (!is_systeminfo(type)){
        return false;
    }
    this->type = type;
    this->timer_fd_ = -1;
    this->running_ = false;

    return true;
}

bool SystemInfo::Start(int interval_ms){
    if (timer_fd_ != -1){
        std::cerr << "Timer already running!" << std::endl;
        return false;
    }
    timer_fd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (timer_fd_ == -1) {
        std::cerr << "Failed to create timerfd!" << std::endl;
        return false;
    }

    struct itimerspec new_value {};
    new_value.it_value.tv_sec = interval_ms / 1000;
    new_value.it_value.tv_nsec = (interval_ms % 1000) * 1000000;
    new_value.it_interval.tv_sec = new_value.it_value.tv_sec;
    new_value.it_interval.tv_nsec = new_value.it_value.tv_nsec;

    if (timerfd_settime(timer_fd_, 0, &new_value, nullptr) == -1) {
        std::cerr << "Failed to set timerfd!" << std::endl;
        close(timer_fd_);
        timer_fd_ = -1;
        return false;
    }

    this->running_ = true;
    this->sysinfo_thread = std::thread(&SystemInfo::run,this);
    return true;
}

bool SystemInfo::Stop(){
    this->running_ = false;
    if (this->sysinfo_thread.joinable()){
        this->sysinfo_thread.join();
    }
    return true;
}

auto SystemInfo::readSysInfo(){
    switch(this->type){
        case systemInfo::CPU_TEMP:{
            std::ifstream file("/sys/class/thermal/thermal_zone0/temp");
            double temp;

            if (file.is_open()){
                file >> temp;
                file.close();
                temp/=1000.0;
                std::cout << "Value in readSysInfo is: " << temp << std::endl;
                return temp;
            }
            else{
                std::cerr << "无法读取温度文件！" << std::endl;
            }
        }
        // case systemInfo::GPU_TEMP:{
            
        // }

        case systemInfo::CPU_USAGE:{
            cputimes_1 = getCpuTimes();
            delay.delay_ms(1000);
            cputimes_2 = getCpuTimes();
            uint64_t total_diff = cputimes_2.total() - cputimes_1.total();
            uint64_t idle_diff = cputimes_2.idle_total() - cputimes_1.idle_total();
            double usage = (1.0 - (double)idle_diff / total_diff) * 100.0;
            return usage;
        }
        
        case systemInfo::GPU_USAGE:
        case systemInfo::RAM_USAGE:
        case systemInfo::SWAP_USAGE:
            break;
            
    }
}


void SystemInfo::timerEvent(){
    for (auto &sci : sysinfoCallbackInterfaces){
        double temp = readSysInfo();
        sci->hasSampled(temp);
    }
}
void SystemInfo::run(){
    uint64_t expirations;
    while(running_){
        if (read(timer_fd_, &expirations, sizeof(expirations)) > 0){
            this->timerEvent();
        } 
    }
}
