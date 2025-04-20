#include "Communication.h"
#include "Serial.h"
#include "Control.h"
#include <atomic>
#include <cstdint>
#include "delay.h"

std::atomic_bool running = true;

void feedback_thread(Control& ctl){
    Delay delay;
    std::cout << "feedback is running" << std::endl;
    while(running){
        for (uint8_t id = 1; id <= 6; ++id){
            ctl.FeedBack(id);
        }
        // std::this_thread::sleep_for(std::chrono::milliseconds(1));
        delay.delay_ms(1);
    }
}

int main(){
    Control test(1);
    Delay delay;
    if (!test.Begin(115200, "/dev/ttyUSB0")){
        std::cerr << "串口打开失败！" << std::endl;
        return -1;
    }

    
    std::thread th([&]{
        while(running){
            test.FeedBack(5);
            delay.delay_ms(500);
            int pos = test.ReadPos(5);
            int speed = test.ReadSpeed(5);
            int temp = test.ReadTemper(5);

            std::cout << "舵机当前角度: " << pos << std::endl;
            std::cout << "舵机当前速度: " << speed << std::endl;
            std::cout << "舵机温度: " << temp << std::endl;
        }
    });

    // test.WritePos(0, 512, 500);
    // delay.delay_ms(1000);
    std::cout << "Press Enter to stop...\n";
    std::cin.get();

    running = false;
    if (th.joinable()) 
        th.join();
    test.End();
    return 0;
}