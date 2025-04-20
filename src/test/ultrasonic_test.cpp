#include "Ultrasonic.h"
#include "delay.h"
#include <atomic>
#include <thread>

int main(){
    Ultrasonic test_ult;
    DistanceCalculator dist_cal;
    Delay delay;
    if (!test_ult.init()){
        std::cout << "Ultrasonic not init!" << std::endl;
        return 0;
    }
    test_ult.registerCallback(&dist_cal);
    test_ult.start();
    std::atomic_bool running = true;
    std::thread test_thread([&]{
        while(running){
            std::cout << "Distance is: " << test_ult.getDistance() << " cm" << std::endl;
            delay.delay_ms(100);
        }
    });
    std::cout << "Press Enter to stop...\n";
    std::cin.get();
    running = false;
    test_thread.join();
    test_ult.stop();
    return 1;
}