#ifndef _MOTOR_H_
#define _MOTOR_H_

#include "PCA9685.h"
#include <cstdint>

#define SERVO_MIN 150
#define SERVO_MAX 600


class Motor{
public:
    Motor(){};
    ~Motor(){};
    PCA9685 pwm = PCA9685(); 

    bool init();
    void Forward();
    void Backward();
    void Stop();
    void TurnLeft();
    void TurnRight();
    void FastForward();
    void FastBackward();
    void FastTurnLeft();
    void FastTurnRight();
    void setServoPulse(uint8_t channel, double pulse);
    void Wakeup();
    void Sleep();
private:
    uint8_t MOTOR_RIGHT_1 = 14;
    uint8_t MOTOR_RIGHT_2 = 15;

    uint8_t MOTOR_LEFT_1  = 12;
    uint8_t MOTOR_LEFT_2  = 13;



};


#endif