#ifndef _MOTOR_H_
#define _MOTOR_H_

#include "../PCA9685/PCA9685.h"
#include <cstdint>

#define SERVO_MIN 150
#define SERVO_MAX 600
#define CAMERA_SERVO_CHANNEL 4 //相機馬達通道


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

    void CameraUp();
    void CameraDown();
    void setServoAngle(uint8_t channel, int angle);
    void cleanServoAngle();
private:
    uint8_t MOTOR_RIGHT_1 = 14;
    uint8_t MOTOR_RIGHT_2 = 15;

    uint8_t MOTOR_LEFT_1  = 12;
    uint8_t MOTOR_LEFT_2  = 13;

    // uint8_t MOTOR_RIGHT_1 = 10;
    // uint8_t MOTOR_RIGHT_2 = 11;

    // uint8_t MOTOR_LEFT_1  = 8;
    // uint8_t MOTOR_LEFT_2  = 9;

    int camera_angle = 90;  //相機起始位置
};


#endif