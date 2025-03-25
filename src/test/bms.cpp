#include "bms.h"
#include <cstdint>

int main(){
    BMS bms;
    bms.start();
    std::cout << "Press Enter to stop...\n";
    std::cin.get();
    bms.stop();
    return 0;
}
