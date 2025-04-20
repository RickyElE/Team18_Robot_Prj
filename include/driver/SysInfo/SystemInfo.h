#ifndef __SYSTEM_INFO__
#define __SYSTEM_INFO__

#include <atomic>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/timerfd.h>
#include <thread>
#include <vector>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include "delay.h"

typedef enum{
    CPU_TEMP = 0,
    CPU_USAGE,
    GPU_USAGE,
    // GPU_TEMP,
    RAM_USAGE,
    SWAP_USAGE
}systemInfo;

struct CpuTimes{
    uint64_t user, nice, system, idle, iowait, irq, softirq, steal;
    uint64_t total() const {
        return user + nice + system + idle + iowait + irq + softirq + steal;
    }
    uint64_t idle_total() const {
        return idle + iowait;
    }
};


class SystemInfo{
public:
    SystemInfo(){};
    ~SystemInfo(){};
    bool Init(systemInfo type);
    bool Start(int interval_ms = 1000);
    bool Stop();
    void run();
    double getCPUTempture(){
        return this->tempture;
    }

    struct SysInfoCallbackInterface{
        virtual void hasSampled(double value) = 0;
    };

    void registerCallback(SysInfoCallbackInterface* cb){
        sysinfoCallbackInterfaces.push_back(cb);
    }

    void timerEvent();

    bool is_systeminfo(systemInfo type){
        switch(type){
            case systemInfo::CPU_TEMP:
            // case systemInfo::GPU_TEMP:
            case systemInfo::CPU_USAGE:
            case systemInfo::GPU_USAGE:
            case systemInfo::RAM_USAGE:
            case systemInfo::SWAP_USAGE:
                return true;
            default:
                return false;
        }
    }
    
    systemInfo getSystemInfoType(){
        return type;
    }
    auto readSysInfo();
    void setCPUTemputure(double temp){
        this->tempture = temp;
    }

    void setCpuUsage(double usage){
        this->cpuUsage = usage;
    }

    double getCpuUsage(){
        return this->cpuUsage;
    }

    CpuTimes getCpuTimes(){
        std::ifstream file("/proc/stat");
        std::string cpu_label;
        CpuTimes times{};

        if (file.is_open()) {
            file >> cpu_label >> times.user >> times.nice >> times.system >> times.idle
                >> times.iowait >> times.irq >> times.softirq >> times.steal;
            file.close();
        }

        return times;
    }

private:
    systemInfo type;

    std::vector<SysInfoCallbackInterface*> sysinfoCallbackInterfaces;
    std::thread sysinfo_thread;
    std::atomic_bool running_;
    int timer_fd_;
    uint64_t value;
    double tempture;
    double cpuUsage;

    CpuTimes cputimes_1;
    CpuTimes cputimes_2;
    Delay delay;

};

class Mycallback:public SystemInfo::SysInfoCallbackInterface{
    public:
        SystemInfo *systeminfo = nullptr;
        virtual void hasSampled(double value) override{
            if (nullptr != systeminfo){
                if (systeminfo->getSystemInfoType() == systemInfo::CPU_TEMP){
                    systeminfo->setCPUTemputure(value);
                }
                else
                if  (systeminfo->getSystemInfoType() == systemInfo::CPU_USAGE){
                    systeminfo->setCpuUsage(value);
                }
            }
        }
    };


#endif