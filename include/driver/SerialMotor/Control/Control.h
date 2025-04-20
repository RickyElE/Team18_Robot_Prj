#ifndef __CONTROL_H__
#define __CONTROL_H__

#include "Serial.h"
#include <cstdint>

#define	SERIAL_1M       0
#define	SERIAL_0_5M     1
#define	SERIAL_250K     2
#define	SERIAL_128K     3
#define	SERIAL_115200   4
#define	SERIAL_76800    5
#define	SERIAL_57600    6
#define	SERIAL_38400    7

//内存表定义
//-------EPROM(只读)--------
#define SERIAL_VERSION_L 3
#define SERIAL_VERSION_H 4

//-------EPROM(读写)--------
#define SERIAL_ID 5
#define SERIAL_BAUD_RATE 6
#define SERIAL_MIN_ANGLE_LIMIT_L 9
#define SERIAL_MIN_ANGLE_LIMIT_H 10
#define SERIAL_MAX_ANGLE_LIMIT_L 11
#define SERIAL_MAX_ANGLE_LIMIT_H 12
#define SERIAL_CW_DEAD 26
#define SERIAL_CCW_DEAD 27

//-------SRAM(读写)--------
#define SERIAL_TORQUE_ENABLE 40
#define SERIAL_GOAL_POSITION_L 42
#define SERIAL_GOAL_POSITION_H 43
#define SERIAL_GOAL_TIME_L 44
#define SERIAL_GOAL_TIME_H 45
#define SERIAL_GOAL_SPEED_L 46
#define SERIAL_GOAL_SPEED_H 47
#define SERIAL_LOCK 48

//-------SRAM(只读)--------
#define SERIAL_PRESENT_POSITION_L 56
#define SERIAL_PRESENT_POSITION_H 57
#define SERIAL_PRESENT_SPEED_L 58
#define SERIAL_PRESENT_SPEED_H 59
#define SERIAL_PRESENT_LOAD_L 60
#define SERIAL_PRESENT_LOAD_H 61
#define SERIAL_PRESENT_VOLTAGE 62
#define SERIAL_PRESENT_TEMPERATURE 63
#define SERIAL_MOVING 66
#define SERIAL_PRESENT_CURRENT_L 69
#define SERIAL_PRESENT_CURRENT_H 70

class Control : public Serial{
public:
    Control();
    Control(uint8_t end);
    Control(uint8_t end, uint8_t level);
    virtual int WritePos(uint8_t Id, uint16_t position, uint16_t time, uint16_t speed = 0);//普通写单个舵机位置指令
	virtual int RegWritePos(uint8_t Id, uint16_t position, uint16_t time, uint16_t speed = 0);//异步写单个舵机位置指令(RegWriteAction生效)
	virtual void SyncWritePos(uint8_t Id[], uint8_t IDN, uint16_t position[], uint16_t time[], uint16_t speed[]);//同步写多个舵机位置指令
	virtual int PWMMode(uint8_t Id);//PWM输出模式
	virtual int WritePWM(uint8_t Id, int16_t pwmOut);//PWM输出模式指令
	virtual int EnableTorque(uint8_t Id, uint8_t Enable);//扭矩控制指令
	virtual int UnLockEprom(uint8_t Id);//eprom解锁
	virtual int LockEprom(uint8_t Id);//eprom加锁
	virtual int FeedBack(int Id);//反馈舵机信息
	virtual int ReadPos(int Id);//读位置
	virtual int ReadSpeed(int Id);//读速度
	virtual int ReadLoad(int Id);//读输出至电机的电压百分比(0~1000)
	virtual int ReadVoltage(int Id);//读电压
	virtual int ReadTemper(int Id);//读温度
	virtual int ReadMove(int Id);//读移动状态
	virtual int ReadCurrent(int Id);//读电流

private:
    uint8_t mem[SERIAL_PRESENT_CURRENT_H - SERIAL_PRESENT_POSITION_L + 1];
};



#endif