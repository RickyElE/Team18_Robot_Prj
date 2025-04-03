#include "SysInfo.h"

#include <thread>

template<typename INFOTYPE>
bool SysInfo<INFOTYPE>::start(){
    timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (timer_fd == -1){
        std::cerr << "Failed to create timerfd!" << std::endl;
        return false;
    }

    struct itimerspec new_value {};
    int interval_ms = 1000;
    new_value.it_value.tv_sec = interval_ms / 1000;
    new_value.it_value.tv_nsec = (interval_ms % 1000) * 1000000;
    new_value.it_interval.tv_sec = new_value.it_value.tv_sec;
    new_value.it_interval.tv_nsec = new_value.it_value.tv_nsec;

    if (timerfd_settime(timer_fd, 0, &new_value, nullptr) == -1) {
        std::cerr << "Failed to set timerfd!" << std::endl;
        close(timer_fd);
        timer_fd = -1;
        return false;
    }
    running_ = true;
    worker_thread = std::thread(worker,this);
    return true;
}


template<typename INFOTYPE>
bool SysInfo<INFOTYPE>::stop(){
    running_ = false;
    if (worker_thread.joinable()){
        worker_thread.join();
    }
    close(timer_fd);
    timer_fd = -1;
}


template<typename INFOTYPE>
void SysInfo<INFOTYPE>::worker(){
    uint64_t expirations;
    while(running_){
        if (read(timer_fd, &expirations, sizeof(expirations)) > 0){
            file = std::ifstream(PATH);
            if(file.is_open()){
                std::string temp;
                std::getline(file,temp);
                for (auto &sci : sysinfo_callbackinterfaces){
                    sci->hasSampled(temp, type);
                }
            }
        }
    }
}