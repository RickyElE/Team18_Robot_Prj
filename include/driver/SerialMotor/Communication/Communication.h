#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include "Command_Group.h"
#include <cstdint>
#include <stdint.h>
#include <vector>
#include <functional>
#include <unordered_map>

class Communication{
public:
    Communication();

    Communication(uint8_t end);

    Communication(uint8_t end, uint8_t level);

    int GenWrite(uint8_t Id, uint8_t memAddr, uint8_t *nDat, uint8_t nLen);

	int RegWrite(uint8_t Id, uint8_t memAddr, uint8_t *nDat, uint8_t nLen);

	int RegWriteAction(uint8_t Id = 0xfe);

	void SyncWrite(uint8_t Id[], uint8_t IdN, uint8_t memAddr, uint8_t *nDat, uint8_t nLen);

	int WriteByte(uint8_t Id, uint8_t memAddr, uint8_t bDat);

	int WriteWord(uint8_t Id, uint8_t memAddr, uint16_t wDat);

	int Read(uint8_t Id, uint8_t memAddr, uint8_t *nData, uint8_t nLen);

	int ReadByte(uint8_t Id, uint8_t memAddr);

	int ReadWord(uint8_t Id, uint8_t memAddr);

	int Ping(uint8_t Id);

	int syncReadPacketTx(uint8_t Id[], uint8_t IdN, uint8_t memAddr, uint8_t nLen);

	int syncReadPacketRx(uint8_t Id, uint8_t *nDat);

	int syncReadRxPacketToByte();

	int syncReadRxPacketToWord(uint8_t negBit=0);

	void syncReadBegin(uint8_t IdN, uint8_t rxLen);

	void syncReadEnd();

	void feedBytes(const uint8_t* data, int len);

	int16_t getCachedPosition(uint8_t id) const;
    int16_t getCachedSpeed   (uint8_t id) const;
    int16_t getCachedLoad    (uint8_t id) const;
    uint8_t getCachedVoltage (uint8_t id) const;
    uint8_t getCachedTemperature(uint8_t id) const;
    uint8_t getCachedMoving  (uint8_t id) const;
    int16_t getCachedCurrent (uint8_t id) const; 

	using PosCb = std::function<void(uint8_t id,int16_t raw)>;
    using SpeedCb = std::function<void(uint8_t id,int16_t raw)>;
    using LoadCb = std::function<void(uint8_t id,int16_t raw)>;
    using VoltCb = std::function<void(uint8_t id,uint8_t raw)>;
    using TempCb = std::function<void(uint8_t id,uint8_t raw)>;
    using MovingCb = std::function<void(uint8_t id,uint8_t raw)>;
    using CurrCb = std::function<void(uint8_t id,int16_t raw)>;

    void setPositionCallback(PosCb cb);
    void setSpeedCallback   (SpeedCb cb);
    void setLoadCallback    (LoadCb cb);
    void setVoltageCallback (VoltCb cb);
    void setTemperatureCallback(TempCb cb);
    void setMovingCallback  (MovingCb cb);
    void setCurrentCallback (CurrCb cb);

public:
	uint8_t	level;
	uint8_t	end;
	uint8_t	error;
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

	void Short2Char(uint8_t *dataL, uint8_t* dataH, uint16_t data);

	uint16_t Char2Short(uint8_t dataL, uint8_t dataH);

	int	Ack(uint8_t Id);

private:
    std::vector<uint8_t> rxBuf_;

    std::unordered_map<uint8_t,int16_t> cachePos_;
    std::unordered_map<uint8_t,int16_t> cacheSpd_;
    std::unordered_map<uint8_t,int16_t> cacheLoad_;
    std::unordered_map<uint8_t,uint8_t> cacheVlt_;
    std::unordered_map<uint8_t,uint8_t> cacheTmp_;
    std::unordered_map<uint8_t,uint8_t> cacheMov_;
    std::unordered_map<uint8_t,int16_t> cacheCurr_;

    PosCb    cbPos_;
    SpeedCb  cbSpd_;
    LoadCb   cbLoad_;
    VoltCb   cbVlt_;
    TempCb   cbTmp_;
    MovingCb cbMov_;
    CurrCb   cbCurr_;

    void processPacket(const std::vector<uint8_t>& pkt);
};

#endif