#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>

int main(){
    while(true){
        system("clear");
        std::string temp;
        std::ifstream file("/sys/class/thermal/thermal_zone0/temp");
        if (file.is_open()){
            std::getline(file,temp);
            file.close();
            int temperature = std::stoi(temp)/1000;
            std::cout << "CPU temperature is " << temperature << "°C" << std::endl;
        }
        else{
            std::cout << "Cannot read CPU Temperature!" << std::endl;
        }
        sleep(3);

    }
    return 0;
}