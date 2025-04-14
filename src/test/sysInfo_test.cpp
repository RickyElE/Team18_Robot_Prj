#include "SystemInfo.h"
#include <atomic>
#include <cstdint>
#include <thread>
#include "delay.h"


int main(){
    SystemInfo temp;
    SystemInfo cpu_usage;

    Mycallback mycallback_1;
    Mycallback mycallback_2;
    // double value;
    Delay delay;

    mycallback_1.systeminfo = &temp;
    mycallback_2.systeminfo = &cpu_usage;

    temp.Init(systemInfo::CPU_TEMP);
    cpu_usage.Init(systemInfo::CPU_USAGE);


    temp.registerCallback(&mycallback_1);
    cpu_usage.registerCallback(&mycallback_2);

    temp.Start();
    cpu_usage.Start();
    std::atomic_bool running = true;
    std::thread temp_thread = std::thread([&]{
        while(running){
            std::cout << "value is: " << temp.getCPUTempture() << std::endl;
            // double x = value / 1000.0;
            std::cout << "Temperature is: " << temp.getCPUTempture() << std::endl;
            std::cout << "Cpu Usage is: " << cpu_usage.getCpuUsage() << std::endl;
            delay.delay_ms(1000);
        }
    });
    std::cout << "Press Enter to stop...\n" << std::endl;;
    std::cin.get();
    running = false;
    if (temp_thread.joinable()){
        temp_thread.join();
    }
    temp.Stop();
    cpu_usage.Stop();
    return 0;
}