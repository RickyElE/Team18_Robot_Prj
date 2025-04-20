#include "Serial.h"
#include "Communication.h"
#include <cstdint>
#include <cstdio>
#include <fcntl.h>
#include <termios.h>

Serial::Serial(){
    time_out = 100;
    fd = -1;
    txBufLen = 0;
}

Serial::Serial(uint8_t end) : Communication(end){
    time_out = 100;
    fd = -1;
    txBufLen = 0;
}

Serial::Serial(uint8_t end, uint8_t level) : Communication(end, level){
    time_out = 100;
    fd = -1;
    txBufLen = 0;
}

bool Serial::Begin(int baudRate, const char *serialPort){
    if (fd != -1){
        close(fd);
        fd = -1;
    }

    if (serialPort == NULL){
        return false;
    }

    fd = open(serialPort, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd == -1){
        perror("open:");
        return false;
    }

    fcntl(fd, F_SETFL, FNDELAY);
    tcgetattr(fd, &orgopt);
    tcgetattr(fd, &curopt);

    speed_t BAUDRATE;
    switch(baudRate){
        case 9600:
            BAUDRATE = B9600;
            break;
        case 19200:
            BAUDRATE = B19200;
            break;
        case 38400:
            BAUDRATE = B38400;
            break;
        case 57600:
            BAUDRATE = B57600;
            break;
        case 115200:
            BAUDRATE = B115200;
            break;
        case 500000:
            BAUDRATE = B500000;
            break;
        case 1000000:
            BAUDRATE = B1000000;
            break;
        default:
            BAUDRATE = B115200;
            break;
        }
        cfsetispeed(&curopt, BAUDRATE);
        cfsetospeed(&curopt, BAUDRATE);
    
        printf("serial speed %d\n", baudRate);
        //Mostly 8N1
        curopt.c_cflag &= ~PARENB;
        curopt.c_cflag &= ~CSTOPB;
        curopt.c_cflag &= ~CSIZE;
        curopt.c_cflag |= CS8;
        curopt.c_cflag |= CREAD;
        curopt.c_cflag |= CLOCAL;//disable modem statuc check
        cfmakeraw(&curopt);//make raw mode
        curopt.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
        if(tcsetattr(fd, TCSANOW, &curopt) == 0){
            return true;
        }else{
            perror("tcsetattr:");
            return false;
        }
}

int Serial::SetBaudRate(int baudRate)
{ 
    if(fd==-1){
		return -1;
	}
    tcgetattr(fd, &orgopt);
    tcgetattr(fd, &curopt);
    speed_t BAUDRATE;
    switch(baudRate){
    case 9600:
        BAUDRATE = B9600;
        break;
    case 19200:
        BAUDRATE = B19200;
        break;
    case 38400:
        BAUDRATE = B38400;
        break;
    case 57600:
        BAUDRATE = B57600;
        break;
    case 115200:
        BAUDRATE = B115200;
        break;
    case 230400:
        BAUDRATE = B230400;
	break;
    case 500000:
        BAUDRATE = B500000;
        break;
    default:
        break;
    }
    cfsetispeed(&curopt, BAUDRATE);
    cfsetospeed(&curopt, BAUDRATE);
    return 1;
}

int Serial::ReadSerial(uint8_t *nDat, int nLen){
    int fs_sel;
    fd_set fs_read;
	int rvLen = 0;

    struct timeval time;

    FD_ZERO(&fs_read);
    FD_SET(fd,&fs_read);

    time.tv_sec = 0;
    time.tv_usec = time_out * 1000;

	while(1){
		fs_sel = select(fd + 1, &fs_read, NULL, NULL, &time);
		if(fs_sel){
			rvLen += read(fd, nDat + rvLen, nLen - rvLen);
			if (rvLen < nLen){
				continue;
			} else {
				return rvLen;
			}
		} else {
			return rvLen;
		}
	}
}

int Serial::Write2Serial(uint8_t *nDat, int nLen)
{
	while(nLen--){
		txBuf[txBufLen++] = *nDat++;
	}
	return txBufLen;
}

int Serial::Write2Serial(uint8_t bDat)
{
	txBuf[txBufLen++] = bDat;
	return txBufLen;
}

void Serial::ReadSerialFlush()
{
	tcflush(fd, TCIFLUSH);
}

void Serial::WriteSerialFlush()
{
	if(txBufLen){
		txBufLen = write(fd, txBuf, txBufLen);
		txBufLen = 0;
	}
}

void Serial::End()
{
	fd = -1;
	close(fd);
}