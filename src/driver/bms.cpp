#include "bms.h"
#include <cstdint>

int main(){
    BMS battery;
    battery.showVoltage();
    battery.showPercentage();
    return 0;
}
