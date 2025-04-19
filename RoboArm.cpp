#include "RoboArm.h"
#include <cmath>     // 提供 sqrt, acos, atan2, cos, sin 等数学函数
#include <utility>   // 提供 std::make_pair
#include "SCSCL.h"
#include <iostream>


// ---------- 1. 把“读取当前位置”封装成单独函数 ----------
void UpdateArmState(SCSCL &sc, ArmState &st)
{
    st.basePos = sc.ReadPos(5);
    st.headPos = sc.ReadPos(8);
}


// ---------- 2. 初始化 ----------
void initial_roboarm(SCSCL &sc, ArmState &st)
{
    u8  ids[2]       = {5, 8};
    u16 positions[2] = {400, 130};
    u16 times[2]     = {0,   0};
    u16 speeds[2]    = {500, 500};

    sc.SyncWritePos(ids, 2, positions, times, speeds);
}


// ---------- 3. 直线前后移动（保持两关节相对位移不变） ----------
void MoveForward(SCSCL &sc, ArmState &st, int delta)
{
    int constantDelta = st.basePos - st.headPos;

    int targetBase = st.basePos + delta;
    int targetHead = targetBase - constantDelta;

    if (targetBase <= 620){
        u8  ids[2]       = {5, 8};
        u16 positions[2] = {(u16)targetBase, (u16)targetHead};
        u16 times[2]     = {0, 0};
        u16 speeds[2]    = {500, 500};
        sc.SyncWritePos(ids, 2, positions, times, speeds);
        st.basePos = targetBase;
        st.headPos = targetHead;
    }
    else{
        u8  ids[2]       = {5, 8};
        u16 positions[2] = {(u16)620, (u16)350};
        u16 times[2]     = {0, 0};
        u16 speeds[2]    = {500, 500};
        sc.SyncWritePos(ids, 2, positions, times, speeds);
        st.basePos = 620;
        st.headPos = 350;
    }

}


void MoveBackward(SCSCL &sc, ArmState &st, int delta)
{
    MoveForward(sc, st, -delta);     
}


// ---------- 4. 抬头 / 低头 ----------
void HeadUp(SCSCL &sc, ArmState &st, int delta, int speed)
{
    int target = st.headPos + delta;
    if (target <= 980){ 
        sc.WritePos(8, (u16)target, 0, speed);
        st.headPos = target;
    }
    else{
        sc.WritePos(8, (u16)980, 0, speed);
        st.headPos = 980;
    };
}

void HeadDown(SCSCL &sc, ArmState &st, int delta, int speed)
{
    int target = st.headPos - delta;
    if (target <= 980){ 
        sc.WritePos(8, (u16)target, 0, speed);
        st.headPos = target;
    }
    else{
        sc.WritePos(8, (u16)980, 0, speed);
        st.headPos = 980;
    };
}




// ---------- 5. 调试打印，随时查看缓存 ----------
void PrintState(const ArmState &st, const char *tag)
{
    printf("%s  base=%d  head=%d  Δ=%d\n",
           tag, st.basePos, st.headPos, st.basePos - st.headPos);
}