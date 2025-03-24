#include "motor.h"

bool Motor::init(){
    if (!pwm.init()){
        std::cerr << "PCA9685 Init Failed!" << std::endl;
        return false;
    }

    pwm.setPWMFreq(50);
    return true;
}

void Motor::Forward(){
    pwm.setPin(MOTOR_LEFT_1, 0);
    pwm.setPin(MOTOR_LEFT_2, 2048);
    pwm.setPin(MOTOR_RIGHT_1, 2048);
    pwm.setPin(MOTOR_RIGHT_2, 0);
}

void Motor::Backward(){
    pwm.setPin(MOTOR_LEFT_1, 2048);
    pwm.setPin(MOTOR_LEFT_2, 0);
    pwm.setPin(MOTOR_RIGHT_1, 0);
    pwm.setPin(MOTOR_RIGHT_2, 2048);
}

void Motor::Stop(){
    pwm.setPin(MOTOR_LEFT_1, 0);
    pwm.setPin(MOTOR_LEFT_2, 0);
    pwm.setPin(MOTOR_RIGHT_1, 0);
    pwm.setPin(MOTOR_RIGHT_2, 0);
}

void Motor::TurnLeft(){
    pwm.setPin(MOTOR_LEFT_1, 2048);
    pwm.setPin(MOTOR_LEFT_2, 0);
    pwm.setPin(MOTOR_RIGHT_1, 2048);
    pwm.setPin(MOTOR_RIGHT_2, 0);
}

void Motor::TurnRight(){
    pwm.setPin(MOTOR_LEFT_1, 0);
    pwm.setPin(MOTOR_LEFT_2, 2048);
    pwm.setPin(MOTOR_RIGHT_1, 0);
    pwm.setPin(MOTOR_RIGHT_2, 2048);
}

void Motor::FastForward(){
    pwm.setPin(MOTOR_LEFT_1, 4095);
    pwm.setPin(MOTOR_LEFT_2, 0);
    pwm.setPin(MOTOR_RIGHT_1, 0);
    pwm.setPin(MOTOR_RIGHT_2, 4095);
}

void Motor::FastBackward(){
    pwm.setPin(MOTOR_LEFT_1, 4095);
    pwm.setPin(MOTOR_LEFT_2, 0);
    pwm.setPin(MOTOR_RIGHT_1, 0);
    pwm.setPin(MOTOR_RIGHT_2, 4095);
}

void Motor::FastTurnLeft(){
    pwm.setPin(MOTOR_LEFT_1, 4095);
    pwm.setPin(MOTOR_LEFT_2, 0);
    pwm.setPin(MOTOR_RIGHT_1, 4095);
    pwm.setPin(MOTOR_RIGHT_2, 0);
}

void Motor::FastTurnRight(){
    pwm.setPin(MOTOR_LEFT_1, 0);
    pwm.setPin(MOTOR_LEFT_2, 4095);
    pwm.setPin(MOTOR_RIGHT_1, 0);
    pwm.setPin(MOTOR_RIGHT_2, 4095);
}