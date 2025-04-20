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

int Control::WritePos(uint8_t Id, uint16_t Position, uint16_t Time, uint16_t Speed)
{
	uint8_t bBuf[6];
	Short2Char(bBuf+0, bBuf+1, Position);
	Short2Char(bBuf+2, bBuf+3, Time);
	Short2Char(bBuf+4, bBuf+5, Speed);
	
	return GenWrite(Id, SERIAL_GOAL_POSITION_L, bBuf, 6);
}

int Control::RegWritePos(uint8_t Id, uint16_t Position, uint16_t Time, uint16_t Speed)
{
	uint8_t bBuf[6];
	Short2Char(bBuf+0, bBuf+1, Position);
	Short2Char(bBuf+2, bBuf+3, Time);
	Short2Char(bBuf+4, bBuf+5, Speed);
	
	return RegWrite(Id, SERIAL_GOAL_POSITION_L, bBuf, 6);
}

void Control::SyncWritePos(uint8_t Id[], uint8_t IDN, uint16_t Position[], uint16_t Time[], uint16_t Speed[])
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
    SyncWrite(Id, IDN, SERIAL_GOAL_POSITION_L, (uint8_t*)offbuf, 6);
}

int Control::PWMMode(uint8_t Id)
{
	uint8_t bBuf[4];
	bBuf[0] = 0;
	bBuf[1] = 0;
	bBuf[2] = 0;
	bBuf[3] = 0;
	return GenWrite(Id, SERIAL_MIN_ANGLE_LIMIT_L, bBuf, 4);	
}

int Control::WritePWM(uint8_t Id, int16_t pwmOut)
{
	if(pwmOut<0){
		pwmOut = -pwmOut;
		pwmOut |= (1<<10);
	}
	uint8_t bBuf[2];
	Short2Char(bBuf+0, bBuf+1, pwmOut);
	
	return GenWrite(Id, SERIAL_GOAL_TIME_L, bBuf, 2);
}

int Control::EnableTorque(uint8_t Id, uint8_t Enable)
{
	return WriteByte(Id, SERIAL_TORQUE_ENABLE, Enable);
}

int Control::UnLockEprom(uint8_t Id)
{
	return WriteByte(Id, SERIAL_LOCK, 0);
}

int Control::LockEprom(uint8_t Id)
{
	return WriteByte(Id, SERIAL_LOCK, 1);
}

int Control::FeedBack(int Id) {
    int nLen = Read(Id, SERIAL_PRESENT_POSITION_L, mem, sizeof(mem));
    if (nLen != sizeof(mem)) {
        err = 1;
        return -1;
    }

    ServoState state;
    state.pos = Char2Short(mem[0], mem[1]);
    state.speed = Char2Short(mem[2], mem[3]);
    state.load = Char2Short(mem[4], mem[5]);
    state.voltage = mem[6];
    state.temperature = mem[7];
    state.moving = mem[10];
    state.current = Char2Short(mem[13], mem[14]);

    {
        std::lock_guard<std::mutex> lock(servo_mutex);
        servo_state_map[Id] = state;
    }

    err = 0;
    return nLen;
}

int Control::ReadPos(int Id) {
    std::lock_guard<std::mutex> lock(servo_mutex);
    if (servo_state_map.count(Id)) return servo_state_map[Id].pos;
    return -1;
}

int Control::ReadSpeed(int Id) {
    std::lock_guard<std::mutex> lock(servo_mutex);
    if (servo_state_map.count(Id)) return servo_state_map[Id].speed;
    return -1;
}

int Control::ReadLoad(int Id) {
    std::lock_guard<std::mutex> lock(servo_mutex);
    if (servo_state_map.count(Id)) return servo_state_map[Id].load;
    return -1;
}

int Control::ReadVoltage(int Id) {
    std::lock_guard<std::mutex> lock(servo_mutex);
    if (servo_state_map.count(Id)) return servo_state_map[Id].voltage;
    return -1;
}

int Control::ReadTemper(int Id) {
    std::lock_guard<std::mutex> lock(servo_mutex);
    if (servo_state_map.count(Id)) return servo_state_map[Id].temperature;
    return -1;
}

int Control::ReadMove(int Id) {
    std::lock_guard<std::mutex> lock(servo_mutex);
    if (servo_state_map.count(Id)) return servo_state_map[Id].moving;
    return -1;
}

int Control::ReadCurrent(int Id) {
    std::lock_guard<std::mutex> lock(servo_mutex);
    if (servo_state_map.count(Id)) return servo_state_map[Id].current;
    return -1;
}