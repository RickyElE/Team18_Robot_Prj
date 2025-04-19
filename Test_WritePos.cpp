#include <iostream>
#include <unistd.h>     // usleep
#include "UserInput.h"  // 你的 getch() 实现
#include "RoboArm.h"    // 包含 ArmState / API
#include "Control.h"      // 仅 main.cpp 需要知道库细节

Control   sc;
ArmState  st;           // 实时姿态缓存

int main(int /*argc*/, char ** /*argv*/)
{
    if (!sc.Begin(115200, "/dev/ttyUSB0")) {
        std::cerr << "初始化 SCSCL 失败！\n";
        return 1;
    }

    // ------- 上电 & 复位 -------
    sc.EnableTorque(0, 1);
    usleep(754 * 1000); 
    sc.EnableTorque(1, 1);
    usleep(200 * 1000);             // 给舵机一点点时间再发指令
    initial_roboarm(sc, st);        // 复位并刷新状态
    usleep(754 * 1000);             // 等动作到位
    UpdateArmState(sc,st);
    usleep(754 * 1000);
    std::cout<<"初始化完成，进入主循环"<<std::endl;
    // ------- 主循环 -------
    while (true) {
        char key = getch();         // 阻塞式读取；若想非阻塞需自行改
        switch (key) {
            case 'w':
                MoveForward(sc, st, 30);
                PrintState(st, "FWD");
                break;

            case 's':
                MoveBackward(sc, st, 30);
                PrintState(st, "BACK");
                break;

            case 'p':
                initial_roboarm(sc, st);
                usleep(1000*1000);
                UpdateArmState(sc,st);
                PrintState(st, "HOME");
                break;

            case 'i':
                HeadUp(sc, st, 30, 400);
                PrintState(st, "UP");
                break;

            case 'k':
                HeadDown(sc, st, 30, 200);
                PrintState(st, "DOWN");
                break;

            case 't':               // 上力矩
                sc.EnableTorque(0, 1);
                usleep(100 * 1000);
                sc.EnableTorque(1, 1);
                break;

            case 'm':               // 断力矩
                sc.EnableTorque(0, 0);
                usleep(100 * 1000);
                sc.EnableTorque(1, 0);
                break;

            case 'q':
                goto EXIT_LOOP;     // 或者 while 条件改成 flag
        }
    }

EXIT_LOOP:
    sc.EnableTorque(0, 0);          // 保险起见关掉
    sc.EnableTorque(1, 0);
    sc.End();
    return 0;
}