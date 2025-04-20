#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include "Command_Group.h"
#include <cstdint>
#include <stdint.h>

class Communication{
public:
    Communication();
    Communication(uint8_t end);
    Communication(uint8_t end, uint8_t level);
    int GenWrite(uint8_t Id, uint8_t memAddr, uint8_t *nDat, uint8_t nLen);//普通写指令
	int RegWrite(uint8_t Id, uint8_t memAddr, uint8_t *nDat, uint8_t nLen);//异步写指令
	int RegWriteAction(uint8_t Id = 0xfe);//异步写执行指令
	void SyncWrite(uint8_t Id[], uint8_t IdN, uint8_t memAddr, uint8_t *nDat, uint8_t nLen);//同步写指令
	int WriteByte(uint8_t Id, uint8_t memAddr, uint8_t bDat);//写1个字节
	int WriteWord(uint8_t Id, uint8_t memAddr, uint16_t wDat);//写2个字节
	int Read(uint8_t Id, uint8_t memAddr, uint8_t *nData, uint8_t nLen);//读指令
	int ReadByte(uint8_t Id, uint8_t memAddr);//读1个字节
	int ReadWord(uint8_t Id, uint8_t memAddr);//读2个字节
	int Ping(uint8_t Id);//Ping指令
	int syncReadPacketTx(uint8_t Id[], uint8_t IdN, uint8_t memAddr, uint8_t nLen);//同步读指令包发送
	int syncReadPacketRx(uint8_t Id, uint8_t *nDat);//同步读返回包解码，成功返回内存字节数，失败返回0
	int syncReadRxPacketToByte();//解码一个字节
	int syncReadRxPacketToWrod(uint8_t negBit=0);//解码两个字节，negBit为方向为，negBit=0表示无方向
	void syncReadBegin(uint8_t IdN, uint8_t rxLen);//同步读开始
	void syncReadEnd();//同步读结束
public:
	uint8_t	level;//舵机返回等级
	uint8_t	end;//处理器大小端结构
	uint8_t	error;//舵机状态
	uint8_t syncReadRxPacketIndex;
	uint8_t syncReadRxPacketLen;
	uint8_t *syncReadRxPacket;
	uint8_t *syncReadRxBuff;
	uint16_t syncReadRxBuffLen;
	uint16_t syncReadRxBuffMax;
    protected:
	virtual int Write2Serial(uint8_t *nDat, int nLen) = 0;
	virtual int ReadSerial(uint8_t *nDat, int nLen) = 0;
	virtual int Write2Serial(uint8_t bDat) = 0;
	virtual void ReadSerialFlush() = 0;
	virtual void WriteSerialFlush() = 0;
protected:
	void Write2Buf(uint8_t Id, uint8_t memAddr, uint8_t *nDat, uint8_t nLen, uint8_t fun);
	void Short2Char(uint8_t *dataL, uint8_t* dataH, uint16_t data);//1个16位数拆分为2个8位数
	uint16_t Char2Short(uint8_t dataL, uint8_t dataH);//2个8位数组合为1个16位数
	int	Ack(uint8_t Id);//返回应答
};

#endif