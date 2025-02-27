#ifndef BMS_H
#define BMS_H

#include <cstdint>
#include <iostream>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

// #define ADS7830_ADDR 0x48
// #define CMD_CH0      0x84
// #define POWER_DOWN   0x03

typedef enum{
    POWER_DOWN_BETWEEN_CONVERSIONS = 0x00,
    INTERNAL_REF_OFF_ADC_ON = 0x01,
    INTERNAL_REF_ON_ADC_OFF = 0x02,
    INTERNAL_REF_ON_ADC_ON  = 0x03
} ad7830PowerDownSelection;

typedef enum {
    DIFF_CH0_CH1 = 0x00, ///< Differential CH0-CH1
    DIFF_CH2_CH3 = 0x01, ///< Differential CH2-CH3
    DIFF_CH4_CH5 = 0x02, ///< Differential CH4-CH5
    DIFF_CH6_CH7 = 0x03, ///< Differential CH6-CH7
    DIFF_CH1_CH0 = 0x04, ///< Differential CH1-CH0
    DIFF_CH3_CH2 = 0x05, ///< Differential CH3-CH2
    DIFF_CH5_CH4 = 0x06, ///< Differential CH5-CH4
    DIFF_CH7_CH6 = 0x07, ///< Differential CH7-CH6
    SINGLE_CH0 = 0x08,   ///< Single-Ended CH0
    SINGLE_CH2 = 0x09,   ///< Single-Ended CH2
    SINGLE_CH4 = 0x0A,   ///< Single-Ended CH4
    SINGLE_CH6 = 0x0B,   ///< Single-Ended CH6
    SINGLE_CH1 = 0x0C,   ///< Single-Ended CH1
    SINGLE_CH3 = 0x0D,   ///< Single-Ended CH3
    SINGLE_CH5 = 0x0E,   ///< Single-Ended CH5
    SINGLE_CH7 = 0x0F    ///< Single-Ended CH7
} ad7830ChannelSelectionControl;

class ADS7830{
private:
    int i2c_fd;
    const char* i2c_device = "/dev/i2c-1";
    static constexpr uint8_t ADS7830_ADDR = 0x48;

public:
    ADS7830(){
        std::cout << "ADS7830 Initial!" << std::endl;
        if ((i2c_fd = open(i2c_device, O_RDWR)) < 0){
            throw std::runtime_error("Failed to open I2C device!");
        }

        if (ioctl(i2c_fd, I2C_SLAVE, ADS7830_ADDR) < 0){
            throw std::runtime_error("Failed to open I2C address!");
        }
    };
    ~ADS7830(){
        close(i2c_fd);
    };

    int readADS7830(int channel, ad7830PowerDownSelection pd = INTERNAL_REF_ON_ADC_ON);
};

int ADS7830::readADS7830(int channel, ad7830PowerDownSelection pd){
    if (channel < 0 | channel > 7){
        throw std::invalid_argument{"Invalid channel: must be 0~7!"};
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

    if (read(i2c_fd, &adc_value, 1) != 1){
        throw std::runtime_error("Failed to read ADC value");
    }

    return static_cast<int>(adc_value) << 8;
}


class BMS : public ADS7830{
private:
    const double vol_max = 6.2;
    double voltage = 0;
    double percentage = 0;
public:
    BMS():ADS7830(){
       std::cout << "BMS Initial!" << std::endl;
    };
    ~BMS(){};
    void showVoltage();
    double getVoltage();
    void showPercentage();
    double getPercentage();
    // protected:
    // uint32_t readADS7830(int fd, int channel);
    
};

void BMS::showVoltage(){
    this->getVoltage();
    std::cout << "Voltage is:" << this->voltage << "V" << std::endl;
}


double BMS::getVoltage(){
    uint16_t raw_value = readADS7830(0);
    this->voltage = (raw_value / 65535.0) * 8.4;
#ifdef DEBUG
    std::cout << "Voltage is:" << voltage << "V" << std::endl;
#endif
    return this->voltage;
}

void BMS::showPercentage(){
    this->percentage = (this->voltage / this->vol_max) * 100;
    std::cout << "Percentage is:" << this->percentage << "%" << std::endl;
}

double BMS::getPercentage(){
    this->percentage = (this->voltage / this->vol_max) * 100;
#ifdef DEBUG
    std::cout << "Percentage is:" << this->percentage << "%" << std::endl;
#endif
    return this->percentage;
}
// uint32_t BMS::readADS7830(int fd, int channel){
//     if (channel < 0 || channel > 7){
//         throw std::invalid_argument{"Invalid channel: must be 0~7!"};
//     }
//     // uint8_t command_byte = (channel << 4) | (POWER_DOWN << 2);
//     uint8_t cmd = 0x84 | ((channel & 0x07) << 4);
//     if(write(fd, &cmd, 1) != 1){
//         std::cerr << "I2C Write Error!\n" << std::endl;
//         return -1;
//     }

//     uint8_t data;
//     if (read(fd, &data, 1) != 1){
//         std::cerr << "I2C Read Error!\n" << std::endl;
//         return -1;
//     }
    
//     return static_cast<int>(data) << 8;
// }


// float BMS::showVoltage(){
//     const char *i2c_dev = "/dev/i2c-1";

//     int fd = open(i2c_dev, O_RDWR);
//     if (fd < 0){
//         std::cerr << "Cannot Open I2C Device!\n" << std::endl;
//         return -1;
//     }

//     if (ioctl(fd, I2C_SLAVE, ADS7830_ADDR) < 0){
//         std::cerr << "Cannot Connect To ADS7830(0x48)!\n" << std::endl;
//         return -1;
//     }

//         uint32_t raw_value = readADS7830(fd, 0);
//         if (raw_value < 0){
//             close(fd);
//             return -1;
//         }
//         std::cout << std::hex << std::showbase << raw_value << std::endl;
//         double voltage = (raw_value / 65535.0)* 8.4;
//         std::cout << "Voltage is: " << voltage << "V" << std::endl;
//     close(fd);
//     return 0;
// }

// double BMS::getVoltage(){
//     const char *i2c_dev = "/dev/i2c-1";

//     int fd = open(i2c_dev, O_RDWR);
//     if (fd < 0){
//         std::cerr << "Cannot Open I2C Device!\n" << std::endl;
//         return -1;
//     }

//     if (ioctl(fd, I2C_SLAVE, ADS7830_ADDR) < 0){
//         std::cerr << "Cannot Connect To ADS7830(0x48)!\n" << std::endl;
//         return -1;
//     }

//     uint32_t raw_value = readADS7830(fd, 0);
//     if (raw_value < 0){
//         close(fd);
//         return -1;
//     }
//     // std::cout << std::hex << std::showbase << raw_value << std::endl;
//     double voltage = (raw_value / 65535.0)* 8.4;
//     std::cout << "Voltage is: " << voltage << "V" << std::endl;
//     close(fd);
//     return voltage;
// }

#endif