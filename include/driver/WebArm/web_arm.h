#ifndef WEB_ARM_H
#define WEB_ARM_H

#include <mutex>
#include "Control.h"
#include "PCA9685.h"



struct ArmState {
    int basePos;
    int headPos;
};

class Arm {
public:
    // 构造：把 sc,pca 引入
    Arm() : sc(), pca(), st{400,130} {}
    ~Arm(){}
    


    bool Initial_Arm();

    // 以下都改成无 pca、无 sc/st 参数
    void UpdateArmState();
    void initial_roboarm();
    void ArmForward(int delta);
    void ArmBackward(int delta);
    void HeadUp(int delta, int speed);
    void HeadDown(int delta, int speed);

    // 保留全局 static 变量，不放到 ArmState
    void RR();       // 手腕右转
    void RL();       // 手腕左转
    void cut();      // 夹紧
    void release();  // 放松

    // WebSocket 映射
    void MoveUpWS();
    void MoveDownWS();
    void ForwardWS();
    void BackwardWS();

    void stop();


private:
    Control sc;    // SCServo
    PCA9685 pca;   // PCA9685
    ArmState st;    // 基座/头部状态
    std::mutex  hw_mutex_;

};

#endif 
