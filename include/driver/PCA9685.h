#ifndef _PCA9685_H_
#define _PCA9685_H_

#include <cstdint>
#include <iostream>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

// default address if ADDR is pulled to GND
#define DEFAULT_PCA9685_ADDRESS 0x5f

class PCA9685{
private:
    int i2c_fd;
    const char* i2c_device = "/dev/i2c-1";
    static constexpr uint8_t PCA9685_ADDR = 0x5f;
public:
    bool writePCA9685(int channel, double var){
        if (channel < 0 || channel > 15){
            throw std::invalid_argument{"Invalid channel: must be 0~15!"};
        }

        if ((i2c_fd = open(i2c_device, O_RDWR)) < 0){
            throw std::runtime_error("Failed to open I2C device!");
        }
    
        if (ioctl(i2c_fd, I2C_SLAVE, PCA9685_ADDR) < 0){
            throw std::runtime_error("Failed to open I2C address!");
        }

        uint8_t command_byte = 0;
    
        if (channel % 2 == 0){
            command_byte |= (SINGLE_CH0 + (channel / 2));
        }
        else{
            command_byte |= (SINGLE_CH1 + ((channel - 1) / 2));
        }
    #ifdef DEBUG
        std::cout << std::hex << "command_byte after DIFF Selection is: " << (uint16_t*)command_byte << std::endl;
    #endif
        command_byte <<= 4;
    
        command_byte |= (pd << 2);
    #ifdef DEBUG
        std::cout << std::hex << "command_byte after pd Selection is: " << (uint16_t*)command_byte << std::endl;
    #endif
        uint8_t adc_value;
        if (write(i2c_fd, &command_byte, 1) != 1){
            throw std::runtime_error("Failed to write to ADC");
        }
    }
};


#endif