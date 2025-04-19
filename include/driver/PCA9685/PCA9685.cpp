#include "PCA9685.h"
#include <cstdint>
#include <fcntl.h>
#include <iostream>
#include <linux/i2c-dev.h>
#include <stdexcept>
#include <string>
#include <sys/ioctl.h>

PCA9685::PCA9685(): PCA9685_ADDR(DEFAULT_PCA9685_ADDRESS){}

PCA9685::PCA9685(const uint8_t addr):PCA9685_ADDR(addr){}

bool PCA9685::init(){
    i2c_fd = open(i2c_device, O_RDWR);
    if (i2c_fd < 0){
        std::cerr << "Failed to open I2C device: " + std::string(i2c_device) << std::endl;
        return false;

    }

    if (ioctl(i2c_fd, I2C_SLAVE, PCA9685_ADDR) < 0){
        std::cerr << "Failed to set I2C address: " + std::to_string(PCA9685_ADDR) << std::endl;
        return false;
    }
    return true;
}

PCA9685::~PCA9685(){
    if (i2c_fd >= 0){
        close(i2c_fd);
    }
}

uint8_t PCA9685::readPrescale(){
    return readRegister(PRESCALE);
}

void PCA9685::setPWMFreq(double freq){
    if (freq < 1){
        freq = 1;
    }
    if (freq > 3500){
        freq = 3500;
    }

    double prescaler_num = ((oscillator_freq / (freq * 4096)) + 0.5) - 1;
    if (prescaler_num < PRESCALE_MIN){
        prescaler_num = PRESCALE_MIN;
    }
    if (prescaler_num > PRESCALE_MAX){
        prescaler_num = PRESCALE_MAX;
    }

    uint8_t prescaler = (uint8_t)prescaler_num;
    std::cout << "new prescaler is: " << std::to_string(prescaler) << std::endl;
    uint8_t old_mode = readRegister(MODE_1_ADDR);
    std::cout << "old_mode is: " << std::to_string(old_mode) << std::endl;
    uint8_t new_mode = (old_mode & ~MODE_1_RESTART) | MODE_1_SLEEP; // sleep
    std::cout << "Write new mode!" << std::endl;
    writeRegister(MODE_1_ADDR, new_mode);
    std::cout << "Write new prescaler!" << std::endl;
    writeRegister(PRESCALE, prescaler);
    std::cout << "Write old mode!" << std::endl;
    writeRegister(MODE_1_ADDR, old_mode);
    delay.delay_ms(5);
    std::cout << "Write Combined mode!" << std::endl;
    writeRegister(MODE_1_ADDR, old_mode | MODE_1_RESTART | MODE_1_AI);

}

void PCA9685::sleep(){
    uint8_t awake = readRegister(MODE_1_ADDR);
    uint8_t sleep = awake | MODE_1_SLEEP;
    writeRegister(MODE_1_ADDR, sleep);
    delay.delay_ms(5);
}

void PCA9685::wakeup(){
    uint8_t sleep = readRegister(MODE_1_ADDR);
    uint8_t wakeup = sleep & ~MODE_1_SLEEP;
    writeRegister(MODE_1_ADDR, wakeup);
}

void PCA9685::reset(){
    uint8_t oldmode = readRegister(MODE_1_ADDR);
    writeRegister(MODE_1_ADDR, MODE_1_RESTART);
    delay.delay_ms(10);
}

uint16_t PCA9685::getPWM(uint8_t param, bool off){
    // uint8_t buf[2] = {uint8_t(LED0_ON_L + 4 * param), 0};
    // if (off){
    //     buf[0] += 2;
    // }
    return readRegister(LED0_ON_L + 4 * param);
}
uint8_t PCA9685::setPWM(uint8_t param, uint16_t on, uint16_t off){
    uint8_t buf[5];
    buf[0] = LED0_ON_L + 4 * param;
    buf[1] = on;
    buf[2] = on >> 8;
    buf[3] = off;
    buf[4] = off >> 8;

    if (i2c_fd < 0){
        std::cerr << "Invalid I2C device!" << std::endl;
        return -1;
    } 
    if (write(i2c_fd, buf, 5) == 5){
        // std::cerr << "I2C write successful" << std::endl;
        return 0;
    }
    else{
        std::cerr << "I2C write failed" << std::endl;
        return 1;
    }
}

void PCA9685::setPin(uint8_t param, uint16_t val, bool invert){
    val = std::min(val, (uint16_t)4095);
    if (invert){
        if (val == 0){
            setPWM(param, 4096, 0);
        }
        else
        if (val == 4095){
            setPWM(param, 0, 4096);
        }
        else{
            setPWM(param, 0, 4095 - val);
        }
    }
    else{
        if (val == 4095){
            setPWM(param, 4096, 0);
        }
        else
        if (val == 0){
            setPWM(param, 0, 4096);
        }
        else{
            setPWM(param, 0, val);
        }
    }
} 