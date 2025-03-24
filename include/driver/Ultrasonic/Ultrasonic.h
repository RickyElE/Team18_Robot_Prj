#ifndef __ULTRASONIC__
#define __ULTRASONIC__

#include <atomic>
#include <ctime>
#include <gpiod.h>
#include "delay.h"
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>

#define ISR_TIMEOUT 1

class Ultrasonic{
public:
    Ultrasonic(){};
    ~Ultrasonic(){
        stop();
    };

    bool init();
    bool start();
    void stop();
    struct UltrasonicCallbackInterface{
        virtual void hasDistance(gpiod_line_event& e, std::chrono::steady_clock::time_point& c, std::chrono::steady_clock::time_point& f, std::mutex& t, std::atomic_bool& able) = 0;
    };

    void registerCallback(UltrasonicCallbackInterface* cb){
        ultsonCallbackInterfaces.push_back(cb);
    }

    double getDistance(){
        return distance;
    }

private:
    const int _GPIOCHIP0 = 0;
    const int _TRIG_OFFSET = 23;
    const int _ECHO_OFFSET = 24;
    gpiod_chip *chipGPIO_trig = nullptr;
    gpiod_line *lineGPIO_trig = nullptr;

    gpiod_chip *chipGPIO_echo = nullptr;
    gpiod_line *lineGPIO_echo = nullptr;

    Delay delay = Delay();

    std::vector<UltrasonicCallbackInterface*> ultsonCallbackInterfaces;
    
    std::thread ultsonThread;

    std::atomic_bool running;

    void worker();

    bool trigOuput(int value);

    bool reqDistance();

    void echoEvent(gpiod_line_event& event);

    std::atomic_bool able_to_cal = false;

    std::chrono::steady_clock::time_point echo_start_time;

    std::chrono::steady_clock::time_point echo_finish_time;

    std::mutex time_mutex_;


    double distance = 0;

};

class DistanceCalculator:public Ultrasonic::UltrasonicCallbackInterface{
    virtual void hasDistance(gpiod_line_event& e, std::chrono::steady_clock::time_point& s, std::chrono::steady_clock::time_point& f,std::mutex& t, std::atomic_bool& able) override{
        if (e.event_type == GPIOD_LINE_EVENT_RISING_EDGE){
            std::lock_guard<std::mutex> lock(t);
            s = std::chrono::steady_clock::now();
        }
        else
        if (e.event_type == GPIOD_LINE_EVENT_FALLING_EDGE){
            std::lock_guard<std::mutex> lock(t);
            f = std::chrono::steady_clock::now();
            able = true;
        }
    }
};

#endif