/*
舵机出厂速度单位是0.0146rpm，速度为V=1500
*/

#include <iostream>
#include "SCServo.h"
#include "UserInput.h"
#include <unistd.h>

SCSCL sc;

int main(int argc, char **argv)
{
	

	// 使用默认串口 "/dev/ttyUSB0"，可以根据需求修改
    if(!sc.begin(115200, "/dev/ttyUSB0")){
        std::cout << "Failed to init scscl motor!" << std::endl;
        return 0;
    }


	int Servo_ID = 1 ;

	// 初始目标位置（根据实际情况选择合适的初值）
	int pos = 50;
	sc.WritePos(Servo_ID, (u16)pos, 0, 1500);
	usleep(754 * 1000); 

	// 读取反馈信息：调用 FeedBack() 将数据读入内部缓存，然后用 ReadPos() 获取当前位置
	int feedbackPos = sc.ReadPos(Servo_ID);




	std::cout << "Initial position = " << pos << std::endl;
	std::cout << "Feedback current position = " << feedbackPos << std::endl;
	std::cout << "Control instructions:" << std::endl;
	std::cout << "  Press 'w' or 'd' to increase position by 50." << std::endl;
	std::cout << "  Press 'a' or 's' to decrease position by 50." << std::endl;
	std::cout << "  Press 'q' to quit." << std::endl;

	while(1){
		// 阻塞读取一个字符
        char key = getch();
        // 根据按键改变目标位置：每次变化 50
        if(key == 'w' || key == 'd'){
            feedbackPos += 50;
        }
        else if(key == 'a' || key == 's'){
            feedbackPos -= 50;
        }
        else if(key == 'q') { // 按 q 键退出
            break;
		}

		// 将新的目标位置发送给舵机（此处 ID 设置为 5，根据实际情况修改）
        sc.WritePos(Servo_ID, (u16)feedbackPos, 0, 1500);
        //std::cout << "Current position = " << pos << std::endl;
        
        // 延时约 754 毫秒，确保舵机有足够时间运动到目标位置
        usleep(754 * 1000);

		// 读取反馈信息：调用 FeedBack() 将数据读入内部缓存，然后用 ReadPos() 获取当前位置
        if(sc.FeedBack(Servo_ID) != -1){
            int feedbackPos = sc.ReadPos(Servo_ID);
            std::cout << "Feedback current position = " << feedbackPos << std::endl;
        } else {
            std::cerr << "Feedback error." << std::endl;
        }
    }
    
	sc.end();
	return 0;
}

