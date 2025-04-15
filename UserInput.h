
#ifndef USERINPUT_H
#define USERINPUT_H

#include <iostream>
#include <limits>
#include <termios.h>

// getch: 从键盘读取一个单字符，不需回车（适用于Linux/Unix系统）
inline char getch() {
    char buf = 0;
    struct termios old = {0};
    if(tcgetattr(0, &old) < 0)
        perror("tcgetattr()");
    old.c_lflag &= ~ICANON;   // 禁用规范模式
    old.c_lflag &= ~ECHO;     // 关闭回显
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if(tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if(read(0, &buf, 1) < 0)
        perror("read()");
    // 恢复终端属性
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if(tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return buf;
}

inline double AngleCalculate(){
    
}

#endif // USERINPUT_H
