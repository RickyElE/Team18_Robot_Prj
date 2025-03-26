#include "bms.h"
#include "motor.h"
#include "delay.h"
#include "Ultrasonic.h"
#include <atomic>
#include <string>
#include <thread>

void showMainMenu(){
    std::cout << "\n=== Robot Control Menu ===" << std::endl;
    std::cout << "1. Battery Management System (BMS)" << std::endl;
    std::cout << "2. Motor Control" << std::endl;
    std::cout << "3. Ultrasonic Sensor" << std::endl;
    std::cout << "0. Exit" << std::endl;
    std::cout << "Please enter your choice: ";
}

void motorMenu(Motor& motor){
    std::string cmd;
    while(true){
        std::cout << "\n--- Motor Control ---" << std::endl;
        std::cout << "wakeup | sleep | forward | backward | turnleft | turnright | stop | exit" << std::endl;
        std::cout << "> ";
        std::cin >> cmd;

        if (cmd == "forward") motor.FastForward();
        else if (cmd == "backward") motor.FastBackward();
        else if (cmd == "turnleft") motor.TurnLeft();
        else if (cmd == "turnright") motor.TurnRight();
        else if (cmd == "stop") motor.Stop();
        else if (cmd == "wakeup") motor.Wakeup();
        else if (cmd == "sleep") motor.Sleep();
        else if (cmd == "exit") break;
        else std::cout << "Invalid Command" << std::endl;
    }
}

void ultrasonicMenu(Ultrasonic& ult){
    DistanceCalculator dist;
    ult.registerCallback(&dist);
    ult.start();
    std::atomic_bool running = true;
    std::cout << "\n--- Ultrasonic Sensor Started ---\nPress Enter to stop..." << std::endl;
    std::thread ult_thread([&]{
        while(running){
            std::cout << "Distance is : " << ult.getDistance() << " cm" << std::endl;
        }
    });
    std::cin.ignore(); // ignore leftover newline
    std::cin.get();
    if (ult_thread.joinable()){
        running = false;
        ult_thread.join();
        
    }
    ult.stop();
}

void bmsMenu(BMS& bms){
    bms.start();
    std::cout << "\n--- BMS Started ---\nPress Enter to stop..." << std::endl;
    std::cin.ignore(); // ignore leftover newline
    std::cin.get();
    bms.stop();
}

int main(){
    std::string command;
    Delay delay;
    std::atomic_bool main_running = true;

    BMS bms = BMS();

    Motor motor = Motor();
    if (!motor.init())
    {
        std::cout << "Motor has not init, please check!" << std::endl;
        return -1;
    }
    Ultrasonic ult;
    
    if (!ult.init()){
        std::cout << "Ultrasonic has not init, please check!" << std::endl;
        return -1;
    }
    
    bool running = true;
    while(running){
        showMainMenu();
        int choice;
        std::cin >> choice;
        switch (choice) {
            case 1:
                bmsMenu(bms);
                break;
            case 2:
                motorMenu(motor);
                break;
            case 3:
                ultrasonicMenu(ult);
                break;
            case 0:
                running = false;
                break;
            default:
                std::cout << "Invalid choice, please try again." << std::endl;
        }
    }
    std::cout << "Exiting program..." << std::endl;
    return 0;
}