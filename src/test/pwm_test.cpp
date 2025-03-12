#include "PCA9685.h"
#include <cstdint>
#include <string>

int main(){
    PCA9685 pwm_test = PCA9685();
    pwm_test.init();
    // pwm_test.wakeup();
    uint8_t prescale = pwm_test.readPrescale();
    std::cout << "Prescale is : " << std::to_string(prescale) << std::endl;
    pwm_test.setPWMFreq(50);
    prescale = pwm_test.readPrescale();
    std::cout << "Prescale is : " << std::to_string(prescale) << std::endl;
    uint16_t pwm = pwm_test.getPWM(15, false);
    std::cout << "pwm is : " << std::to_string(pwm) << std::endl;
    pwm_test.setPin(14, 0,false);
    pwm_test.setPin(15, 0,false);
    pwm = pwm_test.getPWM(15, false);
    std::cout << "pwm is : " << std::to_string(pwm) << std::endl;
    return 0;
}