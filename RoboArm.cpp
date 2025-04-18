#include "RoboArm.h"
#include <cmath>     // 提供 sqrt, acos, atan2, cos, sin 等数学函数
#include <utility>   // 提供 std::make_pair
#include "SCSCL.h"
#include <iostream>



// 保存关节实时位置的轻量级结构体
struct ArmState {
    int basePos;   // ID0：底座/大臂
    int headPos;   // ID1：末端/头
};

// ---------- 1. 把“读取当前位置”封装成单独函数 ----------
inline void UpdateArmState(SCSCL &sc, ArmState &st)
{
    st.basePos = sc.ReadPos(0);
    st.headPos = sc.ReadPos(1);
}


// ---------- 2. 初始化 ----------
void initial_roboarm(SCSCL &sc, ArmState &st)
{
    u8  ids[2]       = {0, 1};
    u16 positions[2] = {380, 111};
    u16 times[2]     = {0,   0};
    u16 speeds[2]    = {300, 300};

    sc.SyncWritePos(ids, 2, positions, times, speeds);
    UpdateArmState(sc, st);          // 调完立即刷新一次缓存
}


// ---------- 3. 直线前后移动（保持两关节相对位移不变） ----------
void MoveForward(SCSCL &sc, ArmState &st, int delta)
{
    int constantDelta = st.basePos - st.headPos;

    int targetBase = st.basePos + delta;
    int targetHead = targetBase - constantDelta;

    u8  ids[2]       = {0, 1};
    u16 positions[2] = {(u16)targetBase, (u16)targetHead};
    u16 times[2]     = {0, 0};
    u16 speeds[2]    = {300, 500};
    sc.SyncWritePos(ids, 2, positions, times, speeds);

    UpdateArmState(sc, st);          // ⇒ 调完记得刷新
}


void MoveBackward(SCSCL &sc, ArmState &st, int delta)
{
    MoveForward(sc, st, -delta);     
}


// ---------- 4. 抬头 / 低头 ----------
void HeadUp(SCSCL &sc, ArmState &st, int delta, int speed)
{
    int target = st.headPos + delta;
    sc.WritePos(1, (u16)target, 0, speed);

    UpdateArmState(sc, st);
}

void HeadDown(SCSCL &sc, ArmState &st, int delta, int speed)
{
    int target = st.headPos - delta; // ↓ 应该是减
    sc.WritePos(1, (u16)target, 0, speed);

    UpdateArmState(sc, st);
}

// ---------- 5. 调试打印，随时查看缓存 ----------
void PrintState(const ArmState &st, const char *tag = "")
{
    printf("%s  base=%d  head=%d  Δ=%d\n",
           tag, st.basePos, st.headPos, st.basePos - st.headPos);
}