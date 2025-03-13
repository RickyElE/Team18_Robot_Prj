#include "delay.h"
#include <bits/chrono.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

std::string current_time(){
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    auto t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S") << "." << std::setw(3) << std::setfill('0') << ms.count();
    return oss.str();
}

int main(){
    Delay delay;
    std::cout << "["<< current_time() << "]"<< "Delay 2 seconds..." << std::endl;
    delay.delay_ms(10000);
    std::cout << "["<< current_time() << "]"<< "Delay finished!" << std::endl;

    std::cout << "["<< current_time() << "]"<< "Delay 500 microseconds..." << std::endl;
    delay.delay_us(500);
    std::cout << "["<< current_time() << "]"<< "Delay finished!" << std::endl;
}