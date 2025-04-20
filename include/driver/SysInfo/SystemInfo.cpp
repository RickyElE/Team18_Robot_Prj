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
                // std::cout << "Value in readSysInfo is: " << temp << std::endl;
                return temp;
            }
            else{
                std::cerr << "cannot read temperature file!" << std::endl;
            }
        }
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
            break;
        case systemInfo::RAM_USAGE:{
            std::ifstream memInfo("/proc/meminfo");
            std::string line;
            uint64_t memTotal, memAvailable = 0;

            while (std::getline(memInfo, line)){
                if (line.rfind("MemTotal:", 0) == 0){
                    sscanf(line.c_str(), "MemTotal: %ld kB", &memTotal);
                }
                else
                if (line.rfind("MemAvailable: ", 0) == 0){
                    sscanf(line.c_str(), "MemAvailable: %ld kB", &memAvailable);
                    break;
                }
            }

            if (memTotal == 0) return 0.0;

            uint64_t memUsed = memTotal - memAvailable;
            return (double) memUsed / memTotal * 100.0;
        }
        case systemInfo::SWAP_USAGE:{
            std::ifstream meminfo("/proc/meminfo");
            std::string line;
            uint64_t swapTotal = 0, swapFree = 0;
        
            while (std::getline(meminfo, line)) {
                if (line.rfind("SwapTotal:", 0) == 0) {
                    sscanf(line.c_str(), "SwapTotal: %ld kB", &swapTotal);
                } else if (line.rfind("SwapFree:", 0) == 0) {
                    sscanf(line.c_str(), "SwapFree: %ld kB", &swapFree);
                    break;
                }
            }
            // std::cout << "swapTotal: " << swapTotal << " ,swapFree: " << swapFree << std::endl;
            if (swapTotal == 0) return 0.0;
            
            uint64_t swapUsed = swapTotal - swapFree;
            return (double)swapUsed / swapTotal * 100.0;
        }
        default:
            return 0.0;
            
    }
    return 0.0;
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
