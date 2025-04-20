#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "Communication.h"
#include <cstdint>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>

class Serial : public Communication{
public:
    Serial();
    Serial(uint8_t end);
    Serial(uint8_t end, uint8_t level);

protected:
    int Write2Serial(uint8_t *nDat, int nLen);
    int ReadSerial(uint8_t *nDat, int nLen);
    int Write2Serial(uint8_t bDat);
    void ReadSerialFlush();
    void WriteSerialFlush();

public:
    uint64_t time_out;
    int err;

public:
    virtual int GetErr(){  return err;  }
    virtual int SetBaudRate(int baudRate);
    virtual bool Begin(int baudRate, const char* serialPort);
    virtual void End();

protected:
    int fd;
    struct termios orgopt;
    struct termios curopt;
    uint8_t txBuf[255];
    int txBufLen;
};

#endif