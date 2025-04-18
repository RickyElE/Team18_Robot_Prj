#ifndef ROBO_ARM_H_
#define ROBO_ARM_H_

#include <cstdint>      // 用到 u8/u16 时可以改成 <cstdint> 里的 uint8_t/uint16_t
#include <cstdio>       // 若调用 printf，可让使用者自己决定是否包含

// -- 前向声明 -----------------------------------------------------------------
class SCSCL;            // 避免把第三方库头文件暴露给所有调用者

// -- 机械臂实时状态 -----------------------------------------------------------
struct ArmState {
    int basePos;    // 舵机 ID0 当前位置
    int headPos;    // 舵机 ID1 当前位置
};

// -- 位置刷新 -----------------------------------------------------------------
/** 读取舵机实际位置，刷新 ArmState */
void UpdateArmState(SCSCL &sc, ArmState &st);

// -- 初始化 -------------------------------------------------------------------
/** 把机械臂复位到 (380, 111)，并同步 ArmState */
void initial_roboarm(SCSCL &sc, ArmState &st);

// -- 直线前后移动 -------------------------------------------------------------
void MoveForward (SCSCL &sc, ArmState &st, int delta);
void MoveBackward(SCSCL &sc, ArmState &st, int delta);

// -- 抬头 / 低头 --------------------------------------------------------------
/** 末端关节上扬 delta，速度 speed */
void HeadUp  (SCSCL &sc, ArmState &st, int delta, int speed);
/** 末端关节下压 delta，速度 speed */
void HeadDown(SCSCL &sc, ArmState &st, int delta, int speed);

// -- 日志 / 调试 --------------------------------------------------------------
/** 调试输出；可选 tag 标识当前步骤 */
void PrintState(const ArmState &st, const char *tag = "");

#endif  // ROBO_ARM_H_
