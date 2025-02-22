#ifndef BMS_H
#define BMS_H

#include <cstdint>
#include <iostream>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define ADS7830_ADDR 0x48
#define CMD_CH0      0x84

class BMS{
    public:
    BMS(){
        std::cout << "BMS IS INITIAL!" << std::endl;
    };
    ~BMS(){};
    float showVoltage();

    protected:
    uint32_t readADS7830(int fd, int channel);
    
};

#endif