#include "Ultrasonic.h"
#include <bits/types/struct_timespec.h>
#include <cstddef>
#include <gpiod.h>
#include <iostream>
#include <thread>
#include "delay.h"

bool Ultrasonic::init(){
    std::cout << "Initializing the Ultrasonic..." << std::endl;
    int ret = -1;
    chipGPIO_trig = gpiod_chip_open_by_number(_GPIOCHIP0);
    if (chipGPIO_trig == NULL){
        std::cout<< "chipGPIO_trig error"<<std::endl;
        return false;
    }

    lineGPIO_trig = gpiod_chip_get_line(chipGPIO_trig, _TRIG_OFFSET);
    if (lineGPIO_trig == NULL){
        gpiod_chip_close(chipGPIO_trig);
        std::cout <<  "lineGPIO_trig error" << std::endl;
        return false;
    }
    ret = gpiod_line_request_output(lineGPIO_trig, "TRIG", 0);
    if (ret < 0){
        std::cout<< "Request trig as output failed" << std::endl;
        gpiod_line_release(lineGPIO_trig);
        return false;
    }

    chipGPIO_echo = gpiod_chip_open_by_number(_GPIOCHIP0);
    if (chipGPIO_echo == NULL){
        std::cout << "chipGPIO_echo error" << std::endl;
        return false;
    }
    lineGPIO_echo = gpiod_chip_get_line(chipGPIO_echo, _ECHO_OFFSET);
    if (lineGPIO_echo == NULL){
        gpiod_chip_close(chipGPIO_echo);
        std::cout<< "lineGPIO_echo error" << std::endl;
        return false;
    }
    ret = gpiod_line_request_both_edges_events(lineGPIO_echo, "ECHO");
    if (ret < 0){
        std::cout<< "Request echo as input failed" << std::endl;
        gpiod_line_release(lineGPIO_echo);
        return false;
    }
    std::cout << "Ultrasonic Initialized Successfully!" << std::endl;
    return true;
}

bool Ultrasonic::trigOuput(int value = -1){
    if (value < 0){
        std::cout << "Please set 0 or 1!" << std::endl;
        return false;
    }
    if (1 == value){
        int ret = gpiod_line_set_value(lineGPIO_trig,1);
        if (ret < 0){
            std::cout << "lineGPIO_trig set 1 failed" << std::endl;
            return false;
        }
        return true;
    }
    else
    if (0 == value){
        int ret = gpiod_line_set_value(lineGPIO_trig,0);
        if (ret < 0){
            std::cout << "lineGPIO_trig set 0 failed" << std::endl;
            return false;
        }
        return true;
    }
    else{
        std::cout << "Error value: " << value << " ,Please set 0 or 1!" << std::endl;
        return false;
    }
}

bool Ultrasonic::reqDistance(){
    // std::cout << "Requiring Ultrasonic Distance ..." << std::endl;
    bool ret = false;
    ret = trigOuput(0);
    if (!ret) return false;
    delay.delay_ms(200);
    ret = trigOuput(1);
    if (!ret) return false;
    delay.delay_us(10);
    ret = trigOuput(0);
    if (!ret) return false;
    // std::cout << "Has requiring sensor!" << std::endl;
    able2Req = false;
    return true;
}

bool Ultrasonic::start(){
    running = true;
    ultsonThread = std::thread(&Ultrasonic::worker, this);
    return true;
}

void Ultrasonic::echoEvent(gpiod_line_event& event){
    for(auto &uci: ultsonCallbackInterfaces){
        uci->hasDistance(event, echo_start_time, distance,time_mutex_,able2Req);
        // std::cout << "distance is: " << distance << std::endl;
    }
}

void Ultrasonic::worker(){
    reqDistance();
    while(running){
        if (able2Req){
            reqDistance();
        }
        const timespec ts = {ISR_TIMEOUT,0};
        int r = gpiod_line_event_wait(lineGPIO_echo, &ts);
        if (1 == r){
            // std::cout << "Has distance!" << std::endl;
            gpiod_line_event event;
            gpiod_line_event_read(lineGPIO_echo, &event);
            echoEvent(event);
        }
        // std::cout << "Distance is: " << getDistance() << std::endl;
    }
}

Ultrasonic::~Ultrasonic(){
    if (lineGPIO_trig != NULL) gpiod_line_release(lineGPIO_trig);
    if (lineGPIO_echo != NULL) gpiod_line_release(lineGPIO_echo);
    if (chipGPIO_trig != NULL) gpiod_chip_close(chipGPIO_trig);
    if (chipGPIO_echo != NULL) gpiod_chip_close(chipGPIO_echo);
}

void Ultrasonic::stop(){
    running = false;
    if (ultsonThread.joinable()){
        ultsonThread.join();
    }
}
// int main(){
//     gpiod_chip *chipGPIO_trig = nullptr;
//     gpiod_line *lineGPIO_trig = nullptr;

//     gpiod_chip *chipGPIO_echo = nullptr;
//     gpiod_line *lineGPIO_echo = nullptr;

//     Delay delay = Delay();

//     chipGPIO_trig = gpiod_chip_open_by_number(0);
//     if (chipGPIO_trig == NULL){
//         throw "chipGPIO_trig error";
//     }
//     lineGPIO_trig = gpiod_chip_get_line(chipGPIO_trig, 23);
//     if (lineGPIO_trig == NULL){
//         gpiod_chip_close(chipGPIO_trig);
//         throw "lineGPIO_trig error";
//     }
//     int ret_1 = gpiod_line_request_output(lineGPIO_trig, "trig", 0);
//     if (ret_1 < 0){
//         std::cout<< "Request line23 as output failed" << std::endl;
//         gpiod_line_release(lineGPIO_trig);
//         return 0;
//     }


//     chipGPIO_echo = gpiod_chip_open_by_number(0);
//     if (chipGPIO_echo == NULL){
//         throw "chipGPIO_echo error";
//     }
//     lineGPIO_echo = gpiod_chip_get_line(chipGPIO_echo, 24);
//     if (lineGPIO_echo == NULL){
//         gpiod_chip_close(chipGPIO_trig);
//         throw "lineGPIO_echo error";
//     }
//     int ret_2 = gpiod_line_request_input(lineGPIO_echo, "echo");
//     if (ret_2 < 0){
//         std::cout<< "Request line24 as input failed" << std::endl;
//         gpiod_line_release(lineGPIO_echo);
//         return 0;
//     }

//     int ret_3 = gpiod_line_set_value(lineGPIO_trig, 0);
//         if (ret_3 < 0){
//             std::cout << "lineGPIO_trig set 0 failed" << std::endl;
//             gpiod_line_release(lineGPIO_trig);
//             return 0;
//         }
//         delay.delay_ms(200);
//         ret_3 = gpiod_line_set_value(lineGPIO_trig, 1);
//         if (ret_3 < 0){
//             std::cout << "lineGPIO_trig set 1 failed" << std::endl;
//             gpiod_line_release(lineGPIO_trig);
//             return 0;
//         }
//         delay.delay_us(10);
//         ret_3 = gpiod_line_set_value(lineGPIO_trig, 0);
//         if (ret_3 < 0){
//             std::cout << "lineGPIO_trig set 0 failed" << std::endl;
//             gpiod_line_release(lineGPIO_trig);
//             return 0;
//         }
//         int ret4 = -1;
//         int count = 0;
//         do{
//             ret4 = gpiod_line_get_value(lineGPIO_echo);
//             count++;
//         }
//         while(ret4 == 0);
//         std::cout << "count is: " << count << std::endl;
//         double distance = ((count * 342.62 / 2)/10000000) * 100;
//         std::cout << "distance is: " << distance << std::endl;
//         gpiod_line_release(lineGPIO_trig);
//         gpiod_line_release(lineGPIO_echo);
//         gpiod_chip_close(chipGPIO_trig);
//         gpiod_chip_close(chipGPIO_echo);
    



//     return 0;
// }