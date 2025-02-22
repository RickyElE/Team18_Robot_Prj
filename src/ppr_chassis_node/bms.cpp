#include "bms.h"
#include <cstdint>

#define ADS7830_ADDR 0x48
#define CMD_CH0      0x84

uint32_t BMS::readADS7830(int fd, int channel){
    uint8_t cmd = 0x84 | ((channel & 0x07) << 4);
    if(write(fd, &cmd, 1) != 1){
        std::cerr << "I2C Write Error!\n" << std::endl;
        return -1;
    }

    uint8_t data;
    if (read(fd, &data, 1) != 1){
        std::cerr << "I2C Read Error!\n" << std::endl;
        return -1;
    }
    
    return data;
}


float BMS::showVoltage(){
    const char *i2c_dev = "/dev/i2c-1";

    int fd = open(i2c_dev, O_RDWR);
    if (fd < 0){
        std::cerr << "Cannot Open I2C Device!\n" << std::endl;
        return -1;
    }

    if (ioctl(fd, I2C_SLAVE, ADS7830_ADDR) < 0){
        std::cerr << "Cannot Connect To ADS7830(0x48)!\n" << std::endl;
        return -1;
    }

        uint32_t raw_value = readADS7830(fd, 0);
        if (raw_value < 0){
            close(fd);
            return -1;
        }
        std::cout << std::hex << std::showbase << raw_value << std::endl;
        double voltage = (raw_value / 255.0)* 5.0 * 2;
        std::cout << "Voltage is: " << voltage << "V" << std::endl;
    close(fd);
    return 0;
}

