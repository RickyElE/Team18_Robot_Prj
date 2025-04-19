#include "RoboArm.h"
#include "Control.h"



// ---------- 1. 把“读取当前位置”封装成单独函数 ----------
void UpdateArmState(Control &sc, ArmState &st)
{
    st.basePos = sc.ReadPos(5);
    st.headPos = sc.ReadPos(8);
}


// ---------- 2. 初始化 ----------
void initial_roboarm(Control &sc, ArmState &st)
{
    uint8_t  ids[2]       = {5, 8};
    uint16_t positions[2] = {400, 130};
    uint16_t times[2]     = {0,   0};
    uint16_t speeds[2]    = {500, 500};

    sc.SyncWritePos(ids, 2, positions, times, speeds);
}


// ---------- 3. 直线前后移动（保持两关节相对位移不变） ----------
void MoveForward(Control &sc, ArmState &st, int delta)
{
    int constantDelta = st.basePos - st.headPos;

    int targetBase = st.basePos + delta;
    int targetHead = targetBase - constantDelta;

    if (targetBase <= 620){
        uint8_t  ids[2]       = {5, 8};
        uint16_t positions[2] = {static_cast<uint16_t>(targetBase), static_cast<uint16_t>(targetHead)};
        uint16_t times[2]     = {0, 0};
        uint16_t speeds[2]    = {500, 500};
        sc.SyncWritePos(ids, 2, positions, times, speeds);
        st.basePos = targetBase;
        st.headPos = targetHead;
    }
    else{
        uint8_t  ids[2]       = {5, 8};
        uint16_t positions[2] = {static_cast<uint16_t>(620), static_cast<uint16_t>(350)};
        uint16_t times[2]     = {0, 0};
        uint16_t speeds[2]    = {500, 500};
        sc.SyncWritePos(ids, 2, positions, times, speeds);
        st.basePos = 620;
        st.headPos = 350;
    }

}


void MoveBackward(Control &sc, ArmState &st, int delta)
{
    MoveForward(sc, st, -delta);     
}


// ---------- 4. 抬头 / 低头 ----------
void HeadUp(Control &sc, ArmState &st, int delta, int speed)
{
    int target = st.headPos + delta;
    if (target <= 980){ 
        sc.WritePos(8, static_cast<uint16_t>(target), 0, speed);
        st.headPos = target;
    }
    else{
        sc.WritePos(8, static_cast<uint16_t>(980), 0, speed);
        st.headPos = 980;
    }
}


void HeadDown(Control &sc, ArmState &st, int delta, int speed)
{
    int target = st.headPos - delta;
    if (target <= 980){ 
        sc.WritePos(8, static_cast<uint16_t>(target), 0, speed);
        st.headPos = target;
    }
    else{
        sc.WritePos(8, static_cast<uint16_t>(980), 0, speed);
        st.headPos = 980;
    }
}



// ---------- 5. 调试打印，随时查看缓存 ----------
void PrintState(const ArmState &st, const char *tag)
{
    printf("%s  base=%d  head=%d  Δ=%d\n",
           tag, st.basePos, st.headPos, st.basePos - st.headPos);
}