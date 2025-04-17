#include "RoboArm.h"
#include <cmath>     // 提供 sqrt, acos, atan2, cos, sin 等数学函数
#include <utility>   // 提供 std::make_pair
#include "SCSCL.h"
#include <iostream>


void initial_roboarm(SCSCL &sc){
    u8 ids[2] = {0, 1};             // 舵机ID分别为0和1
    u16 positions[2] = {380, 111};     // ID0移动到380，ID1移动到111
    u16 times[2] = {0, 0};            // 两个舵机的运动时间都为0（立即达到目标位置）
    u16 speeds[2] = {100, 100};       // 两个舵机的运动速度都为100
    
    sc.SyncWritePos(ids, 2, positions, times, speeds);
}




double update_math_equ(SCSCL &sc){ 
    int standard_pos1 = 111 ;
    int current_pos1 ;
    current_pos1 = sc.ReadPos(1);
    double const_theta1 = (current_pos1 - standard_pos1)/3.33 ;
    return const_theta1;
}


std::pair<int,int> angleCalculate(SCSCL &sc,int delta_pos0){
    double const_theta1 = update_math_equ(sc);//先调用update_math_equ计算出theta1与水平正方向的偏转角const_theta1


    int current_pos0 = sc.ReadPos(0);//初始化的话应该会读到pos0为380
    int current_pos1 = sc.ReadPos(1);//同上，111
    int target_pos0 ;
    int target_pos1 ;


    double current_theta0 ; //单位：度
    double current_theta1 ; //单位：度
    double target_theta0 ; //单位：度
    double target_theta1 ; //单位：度



    current_theta0 = (current_pos0 - 380.0) / 3.36 + 56 ;//单位：度
    current_theta1 = (current_pos1 - 111.0 ) / 3.33 + 56 ;//单位：度
    

    double delta_theta0 = delta_pos0/3.36 ;//单位：度

    target_theta0 = current_theta0 + delta_theta0 ;//单位：度

    target_theta1 = const_theta1 + target_theta0 ;//由于target_theta0是0号杆与水平线的夹角，根据平行线内错角相同原理，加上之前计算出的theta1与水平正方向的偏转角const_theta1，即可算出target_theta1。单位：度

    target_pos0 = (target_theta0 - 56) * 3.36 + 380;
    target_pos1 = (target_theta1 - 56) * 3.33 + 111;
    
    return {target_pos0,target_pos1};
}

void MoveForward(SCSCL &sc){
    auto pp = angleCalculate(sc,50);
    int target_pos0 = pp.first;
    int target_pos1 = pp.second;

    u8 ids[2]         = { 0, 1 };
    u16 positions[2]  = { (u16)target_pos0, (u16)target_pos1 };
    u16 times[2] = { (u16)0, (u16)0 };
    u16 speeds[2] = { (u16)300,(u16)300};
    sc.SyncWritePos(ids, 2, positions, times, speeds);
}

void MoveBackward(SCSCL &sc){
    auto pp = angleCalculate(sc,-50);
    int target_pos0 = pp.first;
    int target_pos1 = pp.second;

    u8 ids[2]         = { 0, 1 };
    u16 positions[2]  = { (u16)target_pos0, (u16)target_pos1 };
    u16 times[2] = { (u16)0, (u16)0 };
    u16 speeds[2] = { (u16)300,(u16)300};
    sc.SyncWritePos(ids, 2, positions, times, speeds);
}



void HeadUp(SCSCL &sc,int speed){
    int delta_pos1 = 50;
    int current_pos1 = sc.ReadPos(1);
    int target_pos1 = current_pos1 + delta_pos1;
    sc.WritePos(1,(u16)target_pos1,0,speed);
}

void HeadDown(SCSCL &sc,int speed){
    int delta_pos1 = -50;
    int current_pos1 = sc.ReadPos(1);
    int target_pos1 = current_pos1 + delta_pos1;
    sc.WritePos(1,(u16)target_pos1,0,speed);
}








