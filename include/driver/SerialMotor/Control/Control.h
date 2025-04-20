#ifndef __CONTROL_H__
#define __CONTROL_H__

#include "Serial.h"
#include <cstdint>
#include <map>
#include <mutex>

class Control : public Serial{
public:
    Control();
    
	Control(uint8_t end);
    
	Control(uint8_t end, uint8_t level);

	~Control() = default;
    
	virtual int WritePos(uint8_t Id, uint16_t position, uint16_t time, uint16_t speed = 0);
	
	virtual int RegWritePos(uint8_t Id, uint16_t position, uint16_t time, uint16_t speed = 0);
	
	virtual void SyncWritePos(uint8_t Id[], uint8_t IDN, uint16_t position[], uint16_t time[], uint16_t speed[]);
	
	virtual int PWMMode(uint8_t Id);
	
	virtual int WritePWM(uint8_t Id, int16_t pwmOut);
	
	virtual int EnableTorque(uint8_t Id, uint8_t Enable);
	
	virtual int UnLockEprom(uint8_t Id);
	
	virtual int LockEprom(uint8_t Id);
	
	virtual int FeedBack(int Id);
	
	virtual int ReadPos(int Id);
	
	virtual int ReadSpeed(int Id);
	
	virtual int ReadLoad(int Id);
	
	virtual int ReadVoltage(int Id);
	
	virtual int ReadTemper(int Id);
	
	virtual int ReadMove(int Id);
	
	virtual int ReadCurrent(int Id);

private:
    uint8_t mem[SERIAL_PRESENT_CURRENT_H - SERIAL_PRESENT_POSITION_L + 1];

	struct ServoState {
        uint16_t pos;
        int16_t  speed;
        int16_t  load;
        uint8_t  voltage;
        uint8_t  temperature;
        uint8_t  moving;
        int16_t  current;
    };
    std::map<uint8_t, ServoState> servo_state_map;
    std::mutex servo_mutex;
};



#endif