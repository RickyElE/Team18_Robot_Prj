#ifndef _PCA9685_H_
#define _PCA9685_H_

#include <cstdint>
#include <iostream>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "delay.h"

// default address if ADDR is pulled to GND
#define DEFAULT_PCA9685_ADDRESS 0x5f
#define FREQUENCY_OSCILLATOR    25000000
#define PRESCALE_MIN            3
#define PRESCALE_MAX            255

// Addresses
#define MODE_1_ADDR     0x00
#define MODE_2_ADDR     0x01
#define SUBADR_1        0x02
#define SUBADR_2        0x03
#define SUBADR_3        0x04
#define ALL_CALL_ADR    0x05
#define LED0_ON_L       0x06
#define LED0_ON_H       0x07
#define LED0_OFF_L      0x08
#define LED0_OFF_H      0x09

#define LED15_ON_L       0x42
#define LED15_ON_H       0x43
#define LED15_OFF_L      0x44
#define LED15_OFF_H      0x45

#define ALL_LED_ON_L    0xFA
#define ALL_LED_ON_H    0xFB
#define ALL_LED_OFF_L   0xFC
#define ALL_LED_OFF_H   0xFD
#define PRESCALE        0xFE
#define TESTMODE        0xFF

#define MODE_1_ALL_CALL 0x01
#define MODE_1_SUB3     0x02
#define MODE_1_SUB2     0x04
#define MODE_1_SUB1     0x08
#define MODE_1_SLEEP    0x10
#define MODE_1_AI       0x20
#define MODE_1_EXTCLK   0x40
#define MODE_1_RESTART  0x80

#define MODE_2_OUTNE_0  0x01
#define MODE_2_OUTNE_1  0x02
#define MODE_2_OUTDRV   0x04
#define MODE_2_OCH      0x08
#define MODE_2_INVRT    0x10

class PCA9685{
private:
    int i2c_fd = -1;
    const char* i2c_device = "/dev/i2c-1";
    uint8_t PCA9685_ADDR;
    double frequency;
    Delay delay;
    uint32_t oscillator_freq = 25000000;
     //Read Byte
    uint8_t readRegister(uint8_t reg){
        if (i2c_fd < 0){
            std::cerr << "Invalid I2C device!" << std::endl;
            return -1;
        }
        if (write(i2c_fd, &reg, 1) != 1){
            std::cerr << "Failed to write register address" << std::endl;
            return -1;
        }
        // delay.delay_ms(5);
        uint8_t data;
        if (read(i2c_fd, &data, 1) != 1){
            std::cerr << "Failed to read register" << std::endl;
            return -1;
        }

        return data;
    }

    //Write Byte
    void writeRegister(uint8_t reg, uint8_t value){
        if (i2c_fd < 0){
            std::cerr << "Invalid I2C device!" << std::endl;
            return;
        } 
        uint8_t buffer[2] = {reg, value};
        if (write(i2c_fd, buffer, 2) != 2){
            std::cerr << "Failed to write register" << std::endl;
            return;
        }

    }

public:
    PCA9685();
    PCA9685(const uint8_t addr);
    ~PCA9685();
    void setPWMFreq(double freq);
    uint16_t getPWM(uint8_t param, bool off);
    uint8_t setPWM(uint8_t param, uint16_t on ,uint16_t off);
    bool init(); // setting prescale and initialize
    uint8_t readPrescale(void);
    void sleep();
    void wakeup();
    void reset();
    void setPin(uint8_t param, uint16_t val, bool invert = false);
};

#endif