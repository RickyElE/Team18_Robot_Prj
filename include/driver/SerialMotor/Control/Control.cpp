#include "Control.h"
#include "Serial.h"
#include <cstdint>

Control::Control(){
    end = 1;
}

Control::Control(uint8_t end) : Serial(end){
    
}

Control::Control(uint8_t end, uint8_t level) : Serial(end, level){
    
}

int Control::WritePos(uint8_t ID, uint16_t Position, uint16_t Time, uint16_t Speed)
{
	uint8_t bBuf[6];
	Short2Char(bBuf+0, bBuf+1, Position);
	Short2Char(bBuf+2, bBuf+3, Time);
	Short2Char(bBuf+4, bBuf+5, Speed);
	
	return GenWrite(ID, SERIAL_GOAL_POSITION_L, bBuf, 6);
}

int Control::RegWritePos(uint8_t ID, uint16_t Position, uint16_t Time, uint16_t Speed)
{
	uint8_t bBuf[6];
	Short2Char(bBuf+0, bBuf+1, Position);
	Short2Char(bBuf+2, bBuf+3, Time);
	Short2Char(bBuf+4, bBuf+5, Speed);
	
	return RegWrite(ID, SERIAL_GOAL_POSITION_L, bBuf, 6);
}

void Control::SyncWritePos(uint8_t ID[], uint8_t IDN, uint16_t Position[], uint16_t Time[], uint16_t Speed[])
{
    uint8_t offbuf[IDN][6];
    for(uint8_t i = 0; i<IDN; i++){
        uint8_t bBuf[6];
		uint16_t T, V;
		if(Time){
			T = Time[i];
		}else{
			T = 0;
		}
		if(Speed){
			V = Speed[i];
		}else{
			V = 0;
		}
        Short2Char(bBuf+0, bBuf+1, Position[i]);
        Short2Char(bBuf+2, bBuf+3, T);
        Short2Char(bBuf+4, bBuf+5, V);
        memcpy(offbuf[i], bBuf, 6);
    }
    SyncWrite(ID, IDN, SERIAL_GOAL_POSITION_L, (uint8_t*)offbuf, 6);
}

int Control::PWMMode(uint8_t ID)
{
	uint8_t bBuf[4];
	bBuf[0] = 0;
	bBuf[1] = 0;
	bBuf[2] = 0;
	bBuf[3] = 0;
	return GenWrite(ID, SERIAL_MIN_ANGLE_LIMIT_L, bBuf, 4);	
}

int Control::WritePWM(uint8_t ID, int16_t pwmOut)
{
	if(pwmOut<0){
		pwmOut = -pwmOut;
		pwmOut |= (1<<10);
	}
	uint8_t bBuf[2];
	Short2Char(bBuf+0, bBuf+1, pwmOut);
	
	return GenWrite(ID, SERIAL_GOAL_TIME_L, bBuf, 2);
}

int Control::EnableTorque(uint8_t ID, uint8_t Enable)
{
	return WriteByte(ID, SERIAL_TORQUE_ENABLE, Enable);
}

int Control::UnLockEprom(uint8_t ID)
{
	return WriteByte(ID, SERIAL_LOCK, 0);
}

int Control::LockEprom(uint8_t ID)
{
	return WriteByte(ID, SERIAL_LOCK, 1);
}

int Control::FeedBack(int ID)
{
	int nLen = Read(ID, SERIAL_PRESENT_POSITION_L, mem, sizeof(mem));
	if(nLen!=sizeof(mem)){
		err = 1;
		return -1;
	}
	err = 0;
	return nLen;
}
	
int Control::ReadPos(int ID)
{
	int Pos = -1;
	if(ID==-1){
		Pos = mem[SERIAL_PRESENT_POSITION_L-SERIAL_PRESENT_POSITION_L];
		Pos <<= 8;
		Pos |= mem[SERIAL_PRESENT_POSITION_H-SERIAL_PRESENT_POSITION_L];
	}else{
		err = 0;
		Pos = ReadWord(ID, SERIAL_PRESENT_POSITION_L);
		if(Pos==-1){
			err = 1;
		}
	}
	return Pos;
}

int Control::ReadSpeed(int ID)
{
	int Speed = -1;
	if(ID==-1){
		Speed = mem[SERIAL_PRESENT_SPEED_L-SERIAL_PRESENT_POSITION_L];
		Speed <<= 8;
		Speed |= mem[SERIAL_PRESENT_SPEED_H-SERIAL_PRESENT_POSITION_L];
	}else{
		err = 0;
		Speed = ReadWord(ID, SERIAL_PRESENT_SPEED_L);
		if(Speed==-1){
			err = 1;
			return -1;
		}
	}
	if(!err && (Speed&(1<<15))){
		Speed = -(Speed&~(1<<15));
	}	
	return Speed;
}

int Control::ReadLoad(int ID)
{
	int Load = -1;
	if(ID==-1){
		Load = mem[SERIAL_PRESENT_LOAD_L-SERIAL_PRESENT_POSITION_L];
		Load <<= 8;
		Load |= mem[SERIAL_PRESENT_LOAD_H-SERIAL_PRESENT_POSITION_L];
	}else{
		err = 0;
		Load = ReadWord(ID, SERIAL_PRESENT_LOAD_L);
		if(Load==-1){
			err = 1;
		}
	}
	if(!err && (Load&(1<<10))){
		Load = -(Load&~(1<<10));
	}	
	return Load;
}

int Control::ReadVoltage(int ID)
{
	int Voltage = -1;
	if(ID==-1){
		Voltage = mem[SERIAL_PRESENT_VOLTAGE-SERIAL_PRESENT_POSITION_L];	
	}else{
		err = 0;
		Voltage = ReadByte(ID, SERIAL_PRESENT_VOLTAGE);
		if(Voltage==-1){
			err = 1;
		}
	}
	return Voltage;
}

int Control::ReadTemper(int ID)
{
	int Temper = -1;
	if(ID==-1){
		Temper = mem[SERIAL_PRESENT_TEMPERATURE-SERIAL_PRESENT_POSITION_L];	
	}else{
		err = 0;
		Temper = ReadByte(ID, SERIAL_PRESENT_TEMPERATURE);
		if(Temper==-1){
			err = 1;
		}
	}
	return Temper;
}

int Control::ReadMove(int ID)
{
	int Move = -1;
	if(ID==-1){
		Move = mem[SERIAL_MOVING-SERIAL_PRESENT_POSITION_L];	
	}else{
		err = 0;
		Move = ReadByte(ID, SERIAL_MOVING);
		if(Move==-1){
			err = 1;
		}
	}
	return Move;
}

int Control::ReadCurrent(int ID)
{
	int Current = -1;
	if(ID==-1){
		Current = mem[SERIAL_PRESENT_CURRENT_L-SERIAL_PRESENT_POSITION_L];
		Current <<= 8;
		Current |= mem[SERIAL_PRESENT_CURRENT_H-SERIAL_PRESENT_POSITION_L];
	}else{
		err = 0;
		Current = ReadWord(ID, SERIAL_PRESENT_CURRENT_L);
		if(Current==-1){
			err = 1;
			return -1;
		}
	}
	if(!err && (Current&(1<<15))){
		Current = -(Current&~(1<<15));
	}	
	return Current;
}