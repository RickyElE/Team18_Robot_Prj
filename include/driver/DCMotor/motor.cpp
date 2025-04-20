#include "motor.h"

bool Motor::init(){
    if (!pwm.init()){
        std::cerr << "PCA9685 Init Failed!" << std::endl;
        return false;
    }

    pwm.setPWMFreq(50);
    return true;
}
void Motor::Wakeup(){
    pwm.wakeup();
}

void Motor::Sleep(){
    pwm.sleep();
}

void Motor::Forward(){
    std::cout << "Forward" << std::endl;
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


void Motor::CameraUp() {
    // 相機向上移動（減少角度）
    std::cout << "起始角度" << camera_angle << std::endl;
    int new_angle = std::max(0, camera_angle - 20);
    camera_angle = new_angle;
    
    // 設置伺服角度
    setServoAngle(CAMERA_SERVO_CHANNEL, new_angle);
    
    std::cout << "相機上移，當前角度: " << new_angle << std::endl;
}

void Motor::CameraDown() {
    // 相機向下移動（增加角度）
    std::cout << "起始角度" << camera_angle << std::endl;
    int new_angle = std::min(180, camera_angle + 20);
    camera_angle = new_angle;
    
    // 設置伺服角度
    setServoAngle(CAMERA_SERVO_CHANNEL, new_angle);
    
    std::cout << "相機下移，當前角度: " << new_angle << std::endl;
}

void Motor::setServoAngle(uint8_t channel, int angle) {
    // 將角度(0-180)轉換為脈衝寬度(SERVO_MIN-SERVO_MAX)
    int pulse_width = SERVO_MIN + (angle * (SERVO_MAX - SERVO_MIN) / 180);
    
    // 設置PWM脈衝
    pwm.setPWM(channel, 0, pulse_width);
}

void Motor::cleanServoAngle() {
    std::cout << "相機stop: " << std::endl;
    pwm.setPWM(CAMERA_SERVO_CHANNEL, 0, 0);
}

