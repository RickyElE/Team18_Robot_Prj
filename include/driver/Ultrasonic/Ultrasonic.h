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
#include <bits/chrono.h>

#define ISR_TIMEOUT 1

class Ultrasonic{
public:
    Ultrasonic(){};
    ~Ultrasonic(){};

    bool init();
    bool start();
    void stop();
    struct UltrasonicCallbackInterface{
        virtual void hasDistance(gpiod_line_event& e, std::chrono::steady_clock::time_point& c, double& distance, std::mutex& t, std::atomic_bool& able2Req) = 0;
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

    std::chrono::steady_clock::time_point echo_start_time;

    std::chrono::steady_clock::time_point echo_finish_time;

    std::mutex time_mutex_;

    double distance = 0;

    std::atomic_bool able2Req = false;

};

class DistanceCalculator:public Ultrasonic::UltrasonicCallbackInterface{
    virtual void hasDistance(gpiod_line_event& e, std::chrono::steady_clock::time_point& s,double& distance,std::mutex& t, std::atomic_bool& able2Req) override{
        if (e.event_type == GPIOD_LINE_EVENT_RISING_EDGE){
            std::unique_lock<std::mutex> lock(t);
            s = std::chrono::steady_clock::now();
            lock.unlock();
            // std::cout<<"It is rising edge!" << std::endl;
        }
        else
        if (e.event_type == GPIOD_LINE_EVENT_FALLING_EDGE){
            std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
            std::chrono::steady_clock::time_point begin_time;
            {
                std::unique_lock<std::mutex> lock(t);
                begin_time = s;
                lock.unlock();
            }
            std::chrono::duration<double> duration_us = end_time - begin_time;
            double time_us = duration_us.count() * 1e6;
            double distance_cm = ((time_us * 342.62 / 2)/10000000) * 100;
            distance = distance_cm;
            {
                std::unique_lock<std::mutex> lock(t);
                able2Req = true;
                lock.unlock();
            }
            // std::cout << "distance is: " << distance << std::endl;
        }
    }
};

#endif