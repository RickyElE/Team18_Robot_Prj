#include "Communication.h"
#include "Command_Group.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <iterator>

Communication::Communication(){
    level = 1;
    error = 0;
}

Communication::Communication(uint8_t end){
    level = 1;
    this->end = end;
    error = 0;
}

Communication::Communication(uint8_t end, uint8_t level){
    this->level = level;
    this->end = end;
    error = 0;
}

void Communication::Short2Char(uint8_t *dataL, uint8_t *dataH, uint16_t data){
    if (end){
        *dataL = (data >> 8);
        *dataH = (data & 0xff);
    }
    else{
        *dataH = (data >> 8);
        *dataL = (data & 0xff);
    }
}

uint16_t Communication::Char2Short(uint8_t dataL, uint8_t dataH){
    uint16_t data;
    if (end){
        data = dataL;
        data <<= 8;
        data |= dataH;
    }
    else{
        data = dataH;
        data <<= 8;
        data |= dataL; 
    }
    return data;
}

void Communication::Write2Buf(uint8_t Id, uint8_t memAddr, uint8_t *nDat, uint8_t nLen, uint8_t fun){
    uint8_t msgLen = 2;
	uint8_t bBuf[6];
	uint8_t checkSum = 0;
	bBuf[0] = 0xff;
	bBuf[1] = 0xff;
	bBuf[2] = Id;
	bBuf[4] = fun;
	if(nDat){
		msgLen += nLen + 1;
		bBuf[3] = msgLen;
		bBuf[5] = memAddr;
		Write2Serial(bBuf, 6);
		
	}else{
		bBuf[3] = msgLen;
		Write2Serial(bBuf, 5);
	}
	checkSum = Id + msgLen + fun + memAddr;
	uint8_t i = 0;
	if(nDat){
		for(i=0; i<nLen; i++){
			checkSum += nDat[i];
		}
		Write2Serial(nDat, nLen);
	}
	Write2Serial(~checkSum);
}

int Communication::GenWrite(uint8_t Id, uint8_t memAddr, uint8_t *nDat, uint8_t nLen){
    ReadSerialFlush();
    Write2Buf(Id, memAddr, nDat, nLen, WRITE);
    WriteSerialFlush();
    return Ack(Id);
}

int Communication::RegWrite(uint8_t Id, uint8_t memAddr, uint8_t *nDat, uint8_t nLen){
    ReadSerialFlush();
    Write2Buf(Id, memAddr, nDat, nLen, REG_WRITE);
    WriteSerialFlush();
    return Ack(Id);
}

int Communication::RegWriteAction(uint8_t Id){
    ReadSerialFlush();
    Write2Buf(Id, 0, NULL, 0, REG_ACTION);
    WriteSerialFlush();
    return Ack(Id);
}

void Communication::SyncWrite(uint8_t *Id, uint8_t IdN, uint8_t memAddr, uint8_t *nDat, uint8_t nLen){
    ReadSerialFlush();
    uint8_t mesLen = ((nLen+1)*IdN+4);
	uint8_t sum = 0;
	uint8_t bBuf[7];
	bBuf[0] = 0xff;
	bBuf[1] = 0xff;
	bBuf[2] = 0xfe;
	bBuf[3] = mesLen;
	bBuf[4] = SYNC_WRITE;
	bBuf[5] = memAddr;
	bBuf[6] = nLen;
	Write2Serial(bBuf, 7);

	sum = 0xfe + mesLen + SYNC_WRITE + memAddr + nLen;
	uint8_t i, j;
	for(i=0; i<IdN; i++){
		Write2Serial(Id[i]);
		Write2Serial(nDat+i*nLen, nLen);
		sum += Id[i];
		for(j=0; j<nLen; j++){
			sum += nDat[i*nLen+j];
		}
	}
	Write2Serial(~sum);
	WriteSerialFlush();
}

int Communication::WriteByte(uint8_t Id, uint8_t memAddr, uint8_t bDat){
    ReadSerialFlush();
    Write2Buf(Id, memAddr, &bDat, 1, WRITE);
    WriteSerialFlush();
    return Ack(Id);
}

int Communication::WriteWord(uint8_t Id, uint8_t memAddr, uint16_t wDat){
    uint8_t bBuf[2];
    Short2Char(bBuf+0, bBuf+1, wDat);
    ReadSerialFlush();
    Write2Buf(Id, memAddr, bBuf, 2, WRITE);
    WriteSerialFlush();
    return Ack(Id);
}

int Communication::Read(uint8_t Id, uint8_t memAddr, uint8_t *nData, uint8_t nLen){
    ReadSerialFlush();
    Write2Buf(Id, memAddr, &nLen, 1, READ);
    WriteSerialFlush();

    uint8_t bBuf[255];
    uint8_t i;
    uint8_t sum = 0;

    int size = ReadSerial(bBuf, nLen + 6);

    if (size != nLen + 6){
        return 0;
    }

    if (bBuf[0] != 0xff || bBuf[1] != 0xff){
        return 0;
    }

    for (i = 2 ; i < size - 1 ; i++){
        sum += bBuf[i];
    }

    sum = ~sum;

    if (sum != bBuf[size - 1]){
        return 0;
    }

    std::memcpy(nData, bBuf + 5, nLen);

    error = bBuf[4];

    return nLen;
}

int Communication::ReadByte(uint8_t Id, uint8_t memAddr){
    uint8_t bDat;

    int size = Read(Id, memAddr, &bDat, 1);

    if (size != 1){
        return -1;
    }
    else{
        return bDat;
    }
}

int Communication::ReadWord(uint8_t Id, uint8_t memAddr){
    uint8_t nDat[2];
    int size;
    uint16_t wDat;
    size = Read(Id, memAddr, nDat, 2);

    if (size != 2) return -1;

    wDat = Char2Short(nDat[0], nDat[1]);
    return wDat;    
}

int Communication::Ping(uint8_t Id){
    ReadSerialFlush();
    Write2Buf(Id, 0, NULL, 0, PING);
    WriteSerialFlush();
    error = 0;

    uint8_t bBuf[6];
    uint8_t i;
    uint8_t sum = 0;
    int size = ReadSerial(bBuf, 6);
    std::cout << "size is: " << size << std::endl;
    if (size != 6){
        return -1;
    }

    if (bBuf[0] != 0xff || bBuf[1] != 0xff){
        return -1;
    }

    if (bBuf[2] != Id && Id != 0xfe){
        return -1;
    }

    if (bBuf[3] != 2){
        return -1;
    }

    for (i = 2; i < size - 1 ; i++){
        sum += bBuf[i];
    }

    sum = ~sum;

    if (sum != bBuf[size - 1]){
        return -1;
    }

    error = bBuf[2];
    return error;
}

int Communication::Ack(uint8_t Id){
    error = 0;
    if (Id != 0xfe && level){
        uint8_t bBuf[6];
        uint8_t i;
        uint8_t sum = 0;

        int size = ReadSerial(bBuf, 6);
        if (size != 6){
            return 0;
        }

        if (bBuf[0] != 0xff || bBuf[1] != 0xff){
            return 0;
        }

        if (bBuf[2] != Id){
            return 0;
        }

        if (bBuf[3] != 2){
            return 0;
        }

        for (i = 2; i < size - 1; i++){
            sum += bBuf[i];
        }

        sum = ~sum;

        if (sum != bBuf[size - 1]){
            return 0;
        }
        error = bBuf[4];
    }
    return 1;
}

int	Communication::syncReadPacketTx(uint8_t Id[], uint8_t IdN, uint8_t memAddr, uint8_t nLen)
{
	ReadSerialFlush();
	syncReadRxPacketLen = nLen;
	uint8_t sum = (4+0xfe) + IdN + memAddr + nLen + SYNC_READ;
	uint8_t i;
	Write2Serial(0xff);
	Write2Serial(0xff);
	Write2Serial(0xfe);
	Write2Serial(IdN+4);
	Write2Serial(SYNC_READ);
	Write2Serial(memAddr);
	Write2Serial(nLen);
	for(i=0; i<IdN; i++){
		Write2Serial(Id[i]);
		sum += Id[i];
	}
	sum = ~sum;
	Write2Serial(sum);
	WriteSerialFlush();
	
	syncReadRxBuffLen = ReadSerial(syncReadRxBuff, syncReadRxBuffMax);
	return syncReadRxBuffLen;
}

void Communication::syncReadBegin(uint8_t IdN, uint8_t rxLen)
{
	syncReadRxBuffMax = IdN * (rxLen + 6);
	syncReadRxBuff = new uint8_t[syncReadRxBuffMax];
}

void Communication::syncReadEnd()
{
	if(syncReadRxBuff){
		delete syncReadRxBuff;
		syncReadRxBuff = NULL;
	}
}

int Communication::syncReadPacketRx(uint8_t Id, uint8_t *nDat)
{
	uint16_t syncReadRxBuffIndex = 0;
	syncReadRxPacket = nDat;
	syncReadRxPacketIndex = 0;
	while((syncReadRxBuffIndex+6+syncReadRxPacketLen)<=syncReadRxBuffLen){
		uint8_t bBuf[] = {0, 0, 0};
		uint8_t sum = 0;
		while(syncReadRxBuffIndex<syncReadRxBuffLen){
			bBuf[0] = bBuf[1];
			bBuf[1] = bBuf[2];
			bBuf[2] = syncReadRxBuff[syncReadRxBuffIndex++];
			if(bBuf[0]==0xff && bBuf[1]==0xff && bBuf[2]!=0xff){
				break;
			}
		}
		if(bBuf[2]!=Id){
			continue;
		}
		if(syncReadRxBuff[syncReadRxBuffIndex++]!=(syncReadRxPacketLen+2)){
			continue;
		}
		error = syncReadRxBuff[syncReadRxBuffIndex++];
		sum = Id + (syncReadRxPacketLen + 2) + error;
		for(uint8_t i=0; i < syncReadRxPacketLen; i++){
			syncReadRxPacket[i] = syncReadRxBuff[syncReadRxBuffIndex++];
			sum += syncReadRxPacket[i];
		}
		sum = ~sum;
		if(sum != syncReadRxBuff[syncReadRxBuffIndex++]){
			return 0;
		}
		return syncReadRxPacketLen;
	}
	return 0;
}

int Communication::syncReadRxPacketToByte()
{
	if(syncReadRxPacketIndex>=syncReadRxPacketLen){
		return -1;
	}
	return syncReadRxPacket[syncReadRxPacketIndex++];
}

int Communication::syncReadRxPacketToWord(uint8_t negBit)
{
	if((syncReadRxPacketIndex+1)>=syncReadRxPacketLen){
		return -1;
	}
	int word = Char2Short(syncReadRxPacket[syncReadRxPacketIndex], syncReadRxPacket[syncReadRxPacketIndex+1]);
	syncReadRxPacketIndex += 2;
	if(negBit){
		if(word & (1 << negBit)){
			word = -(word & ~(1 << negBit));
		}
	}
	return word;
}

void Communication::feedBytes(const uint8_t* data, int len) {
    rxBuf_.insert(rxBuf_.end(), data, data + len);

    static const uint8_t header[] = {0xff, 0xff};
    while (rxBuf_.size() >= 6) {
        auto it = std::search(
            rxBuf_.begin(), rxBuf_.end(),
            std::begin(header), std::end(header)
        );
        if (it == rxBuf_.end()) {
            rxBuf_.clear();
            break;
        }
        size_t idx = it - rxBuf_.begin();
        if (rxBuf_.size() < idx + 4) break;
        uint8_t length = rxBuf_[idx + 3];
        size_t pktSize = size_t(length) + 4;
        if (rxBuf_.size() < idx + pktSize) break;

        std::vector<uint8_t> pkt(
            rxBuf_.begin() + idx,
            rxBuf_.begin() + idx + pktSize
        );

        rxBuf_.erase(rxBuf_.begin(), rxBuf_.begin() + idx + pktSize);

        processPacket(pkt);
    }
}

void Communication::processPacket(const std::vector<uint8_t>& pkt) {
    if (pkt.size() < 6) return;
    uint8_t id     = pkt[2];
    uint8_t length = pkt[3];
    // uint8_t err = pkt[4];
    uint8_t addr   = pkt[5];
    uint8_t plen   = length - 2;
    const uint8_t* payload = pkt.data() + 6;

    switch (addr) {
      case SERIAL_PRESENT_POSITION_L:
        if (plen == 2) {
            int16_t raw = Char2Short(payload[0], payload[1]);
            cachePos_[id] = raw;
            if (cbPos_) cbPos_(id, raw);
        }
        break;
      case SERIAL_PRESENT_SPEED_L:
        if (plen == 2) {
            int16_t raw = Char2Short(payload[0], payload[1]);
            cacheSpd_[id] = raw;
            if (cbSpd_) cbSpd_(id, raw);
        }
        break;
      case SERIAL_PRESENT_LOAD_L:
        if (plen == 2) {
            int16_t raw = Char2Short(payload[0], payload[1]);
            cacheLoad_[id] = raw;
            if (cbLoad_) cbLoad_(id, raw);
        }
        break;
      case SERIAL_PRESENT_VOLTAGE:
        if (plen == 1) {
            uint8_t raw = payload[0];
            cacheVlt_[id] = raw;
            if (cbVlt_) cbVlt_(id, raw);
        }
        break;
      case SERIAL_PRESENT_TEMPERATURE:
        if (plen == 1) {
            uint8_t raw = payload[0];
            cacheTmp_[id] = raw;
            if (cbTmp_) cbTmp_(id, raw);
        }
        break;
      case SERIAL_MOVING:
        if (plen == 1) {
            uint8_t raw = payload[0];
            cacheMov_[id] = raw;
            if (cbMov_) cbMov_(id, raw);
        }
        break;
      case SERIAL_PRESENT_CURRENT_L:
        if (plen == 2) {
            int16_t raw = Char2Short(payload[0], payload[1]);
            cacheCurr_[id] = raw;
            if (cbCurr_) cbCurr_(id, raw);
        }
        break;
      default:
        break;
    }
}

int16_t Communication::getCachedPosition(uint8_t id) const {
    auto it = cachePos_.find(id);
    return it == cachePos_.end() ? int16_t(-1) : it->second;
}
int16_t Communication::getCachedSpeed(uint8_t id) const {
    auto it = cacheSpd_.find(id);
    return it == cacheSpd_.end() ? int16_t(-1) : it->second;
}
int16_t Communication::getCachedLoad(uint8_t id) const {
    auto it = cacheLoad_.find(id);
    return it == cacheLoad_.end() ? int16_t(-1) : it->second;
}
uint8_t Communication::getCachedVoltage(uint8_t id) const {
    auto it = cacheVlt_.find(id);
    return it == cacheVlt_.end() ? uint8_t(0xFF) : it->second;
}
uint8_t Communication::getCachedTemperature(uint8_t id) const {
    auto it = cacheTmp_.find(id);
    return it == cacheTmp_.end() ? uint8_t(0xFF) : it->second;
}
uint8_t Communication::getCachedMoving(uint8_t id) const {
    auto it = cacheMov_.find(id);
    return it == cacheMov_.end() ? uint8_t(0xFF) : it->second;
}
int16_t Communication::getCachedCurrent(uint8_t id) const {
    auto it = cacheCurr_.find(id);
    return it == cacheCurr_.end() ? int16_t(-1) : it->second;
}

void Communication::setPositionCallback(PosCb cb){ cbPos_ = std::move(cb); }
void Communication::setSpeedCallback(SpeedCb cb)   { cbSpd_ = std::move(cb); }
void Communication::setLoadCallback(LoadCb cb)     { cbLoad_ = std::move(cb); }
void Communication::setVoltageCallback(VoltCb cb)  { cbVlt_ = std::move(cb); }
void Communication::setTemperatureCallback(TempCb cb){ cbTmp_ = std::move(cb); }
void Communication::setMovingCallback(MovingCb cb) { cbMov_ = std::move(cb); }
void Communication::setCurrentCallback(CurrCb cb)  { cbCurr_ = std::move(cb); }