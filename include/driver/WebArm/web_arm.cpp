#include "web_arm.h"
#include <iostream>
#include <algorithm>
#include <cstdint>



namespace {
    double angle_ch2 = 90.0;
    double angle_ch3 = 100.0;
}


// 角度 → tick
static int angleToTicks(double angle){
    const int min_pulse=500, max_pulse=2400, range_deg=180;
    double pulse = min_pulse + (max_pulse - min_pulse)*angle/range_deg;
    return static_cast<int>(pulse/20000.0*4096);
}


// ———— 基本动作，全部用成员 sc/pca/st，无外部传参 ————
void Arm::UpdateArmState(){
    std::lock_guard<std::mutex> lock(hw_mutex_);
    st.basePos = sc.ReadPos(5);
    st.headPos = sc.ReadPos(8);
}

void Arm::initial_roboarm(){
    std::lock_guard<std::mutex> lock(hw_mutex_);
    uint8_t ids[2] = {5,8};
    uint16_t pos[2] = {400,130};
    uint16_t t[2] = {0,0}, s[2] = {500,500};
    sc.SyncWritePos(ids, 2, pos, t, s);
    pca.setPWM(2, 0, angleToTicks(angle_ch2));
    pca.setPWM(3, 0, angleToTicks(angle_ch3));
}

void Arm::ArmForward(int delta){
    std::lock_guard<std::mutex> lock(hw_mutex_);
    int constDelta = st.basePos - st.headPos;
    int tb = st.basePos + delta;
    int th = tb - constDelta;
    if(tb>620){ tb=620; th=350; }
    uint8_t ids[2]={5,8};
    uint16_t pos[2]={uint16_t(tb), uint16_t(th)};
    uint16_t t[2]={0,0}, s[2]={500,500};
    sc.SyncWritePos(ids,2,pos,t,s);
    st.basePos=tb; st.headPos=th;
}

void Arm::ArmBackward(int delta){
    ArmForward(-delta);
}

void Arm::HeadUp(int delta, int speed){
    std::lock_guard<std::mutex> lock(hw_mutex_);
    int tgt = std::min(st.headPos+delta, 980);
    sc.WritePos(8, uint16_t(tgt), 0, speed);
    st.headPos = tgt;
}

void Arm::HeadDown(int delta, int speed){
    std::lock_guard<std::mutex> lock(hw_mutex_);
    int tgt = std::max(st.headPos-delta, 0);
    sc.WritePos(8, uint16_t(tgt), 0, speed);
    st.headPos = tgt;
}

// ———— 夹爪／手腕动作，使用全局 angle_ch2/angle_ch3 ————
void Arm::RR(){
    std::lock_guard<std::mutex> lock(hw_mutex_);
    angle_ch2 = std::min(180.0, angle_ch2+10.0);
    pca.setPWM(2, 0, angleToTicks(angle_ch2));
}

void Arm::RL(){
    std::lock_guard<std::mutex> lock(hw_mutex_);
    angle_ch2 = std::max(0.0, angle_ch2-10.0);
    pca.setPWM(2, 0, angleToTicks(angle_ch2));
}

void Arm::cut(){
    std::lock_guard<std::mutex> lock(hw_mutex_);
    angle_ch3 = std::min(180.0, angle_ch3+50.0);
    pca.setPWM(3, 0, angleToTicks(angle_ch3));

}

void Arm::release(){
    std::lock_guard<std::mutex> lock(hw_mutex_);
    angle_ch3 = std::max(0.0, angle_ch3-50.0);
    pca.setPWM(3, 0, angleToTicks(angle_ch3));

}


void Arm::MoveUpWS()      { HeadUp(30,1500); }
void Arm::MoveDownWS()    { HeadDown(30,1500); }
void Arm::ForwardWS()     { ArmForward(30); }
void Arm::BackwardWS()    { ArmBackward(30); }

void Arm::stop(){
    sc.End();
    sc.EnableTorque(5, 0);
    sc.EnableTorque(8, 0);
    for(int ch=0; ch<16; ++ch){
        pca.setPWM(ch, 0, 0);
    }
}

bool Arm::Initial_Arm(){
    if (!sc.Begin(115200, "/dev/ttyUSB0")) {
        std::cerr << "[错误] SCServo 初始化失败\n";
        return false;
    }
    if (!pca.init()) {
        std::cerr << "[错误] PCA9685 初始化失败\n";
        sc.End();
        return false;
    }
    pca.setPWMFreq(50);
    pca.wakeup();
    initial_roboarm();
    pca.setPWM(2, 0, angleToTicks(angle_ch2));
    pca.setPWM(3, 0, angleToTicks(angle_ch3));
    return true;
}

