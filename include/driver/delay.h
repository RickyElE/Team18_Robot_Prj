#ifndef __DELAY_H__
#define __DELAY_H__

#include <bits/types/struct_itimerspec.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <unistd.h>
#include <sys/timerfd.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

class Delay{
public:
    Delay(){
        tfd_ = timerfd_create(CLOCK_MONOTONIC, 0);
        if (tfd_ == -1){
            perror("timerfd_create failed");
        }
    }
    ~Delay(){
        if (tfd_ != -1){
            close(tfd_);
        }
    }

    void delay_ms(uint64_t milliseconds){
        struct itimerspec new_value{};
        new_value.it_value.tv_sec = milliseconds / 1000;
        new_value.it_value.tv_nsec = (milliseconds % 1000) * 1000000;

        if (timerfd_settime(tfd_, 0, &new_value, NULL) == -1){
            perror("timerfd_settime failed");
            return;
        }

        wait_timer();
    }

    void delay_us(uint64_t microseconds){
        struct itimerspec new_value{};
        new_value.it_value.tv_sec = microseconds / 1000000;
        new_value.it_value.tv_nsec = (microseconds % 1000000) * 1000;

        if (timerfd_settime(tfd_, 0, &new_value, NULL) == -1){
            perror("timerfd_settime failed");
            return;
        }

        wait_timer();
    }
private:
    int tfd_;

    void wait_timer(){
        uint64_t expirations;
        ssize_t s = read(tfd_, &expirations, sizeof(expirations));
        if (s != sizeof(expirations)){
            perror("timerfd read error");
        }
    }
};


#endif