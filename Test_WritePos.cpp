#include <iostream>
#include "UserInput.h"
#include "RoboArm.h"
#include <unistd.h>

SCSCL sc;

int main(int argc, char **argv)
{
    if (!sc.begin(115200, "/dev/ttyUSB0")) {
        std::cerr << "初始化 SCSCL 失败！" << std::endl;
        return 1;
    }

    initial_roboarm(sc);
    sc.EnableTorque(0, 1);
    sc.EnableTorque(1, 1);
    usleep(754 * 1000);

    while (true) {
        char key = getch();
        switch (key) {
            case 'w': MoveForward(sc); break;
            case 's': MoveBackward(sc); break;
            case 'p': initial_roboarm(sc); break;
            case 'i': HeadUp(sc, 200);    break;  // 指定速度
            case 'k': HeadDown(sc, 200);  break;
            case 't':
                sc.EnableTorque(0, 1);
                sc.EnableTorque(1, 1);
                break;
            case 'm':
                sc.EnableTorque(0, 0);
                sc.EnableTorque(1, 0);
                break;
            case 'q': 
                goto EXIT_LOOP;
        }
        usleep(50 * 1000);  // 减少 CPU 占用
    }

EXIT_LOOP:
    sc.end();
    return 0;
}
