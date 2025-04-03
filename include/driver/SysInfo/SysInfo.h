#ifndef __SYS_INFO__
#define __SYS_INFO__

#include "delay.h"
#include <fstream>
#include <atomic>
#include <thread>
#include <vector>
#include <fstream>
#include <sys/timerfd.h>

typedef enum{
    TEMPERATURE = 1,
}SYSINFO_ENUM;

template<typename INFOTYPE>
class SysInfo{
public:
    SysInfo(const char* ADD_PATH, SYSINFO_ENUM t = TEMPERATURE):type(t){
        if (ADD_PATH == NULL){
            throw "The practical path cannot be empty!";
        }
        PATH = *SYSINFO_PATH + *ADD_PATH;
    }
    ~SysInfo();

    struct SysInfoCallbackInterface{
        virtual void hasSampled(INFOTYPE& value, SYSINFO_ENUM t) = 0;
    };
    bool start();
    bool stop();
    void worker();
    void registerCallback(SysInfoCallbackInterface* cb){
        sysinfo_callbackinterfaces.push_back(cb);
    }
    INFOTYPE getData();
private:
    Delay delay;
    const char* SYSINFO_PATH = "/sys/class";
    char* PATH = new char;
    std::ifstream file;
    std::thread worker_thread;
    std::atomic_bool running_;
    int timer_fd;
    INFOTYPE data;
    SYSINFO_ENUM type;
    std::vector<SysInfoCallbackInterface*> sysinfo_callbackinterfaces;
};

template<typename INFOTYPE>
class DatahasSampled:public SysInfo<INFOTYPE>::SysInfoCallbackInterface{
    virtual void hasSampled(INFOTYPE& value, SYSINFO_ENUM t) override {
        
    }
};
#endif