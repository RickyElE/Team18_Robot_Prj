#ifndef ROBO_ARM_H
#define ROBO_ARM_H

#include <cstdint>      // 用到 u8/u16 时可以改成 <cstdint> 里的 uint8_t/uint16_t
#include <cstdio>       // 若调用 printf，可让使用者自己决定是否包含

// -- 前向声明 -----------------------------------------------------------------
class Control;            // 避免把第三方库头文件暴露给所有调用者

// -- 机械臂实时状态 -----------------------------------------------------------
struct ArmState {
    int basePos;    // 舵机 ID5 当前位置
    int headPos;    // 舵机 ID8 当前位置
};

// -- 位置刷新 -----------------------------------------------------------------
/** 读取舵机实际位置，刷新 ArmState */
void UpdateArmState(Control &sc, ArmState &st);

// -- 初始化 -------------------------------------------------------------------
/** 把机械臂复位到 (400,130)，并同步 ArmState */
void initial_roboarm(Control &sc, ArmState &st);

// -- 直线前后移动 -------------------------------------------------------------
void MoveForward (Control &sc, ArmState &st, int delta);
void MoveBackward(Control &sc, ArmState &st, int delta);

// -- 抬头 / 低头 --------------------------------------------------------------
/** 末端关节上扬 delta，速度 speed */
void HeadUp  (Control &sc, ArmState &st, int delta, int speed);
/** 末端关节下压 delta，速度 speed */
void HeadDown(Control &sc, ArmState &st, int delta, int speed);

// -- 日志 / 调试 --------------------------------------------------------------
/** 调试输出；可选 tag 标识当前步骤 */
void PrintState(const ArmState &st, const char *tag = "");

#endif  // ROBO_ARM_H_