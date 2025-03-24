#include "bms.h"
#include "motor.h"

int main(){
    std::string command;
    std::cout << "Please input the command, like bms: " << std::endl;
    std::cin >> command;
    if (command == "bms"){
        BMS bms = BMS();
        bms.start();
        std::cout << "Press Enter to stop...\n";
        std::cin.get();
        bms.stop();
        return 0;
    }
    else
    if (command == "motor"){
        
    }
    else{
        
    }
}