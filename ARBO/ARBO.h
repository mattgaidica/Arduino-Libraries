#ifndef ARBO_H
#define ARBO_H

#include "Arduino.h"

#define SPI_FREQ 1000000
#define maxMem 0x1FFFF

// FRAM op-codes
#define  FRAM_OP_WREN    0x06 //0b00000110
#define  FRAM_OP_WRDI    0x04 //0b00000100
#define  FRAM_OP_RDSR    0x05 //0b00000101
#define  FRAM_OP_WRSR    0x01 //0b00000001
#define  FRAM_OP_READ    0x03 //0b00000011
#define  FRAM_OP_WRITE   0x02 //0b00000010
#define  FRAM_OP_RDID    0x9F //0b10011111
#define  FRAM_OP_FSTRD   0x0B //0b00001011
#define  FRAM_OP_SLEEP   0xB9 //0b10111001
// uint32_t maxMem   0x1FFFF;

// ADS129x op-codes
#define  ADS_OP_RDATAC	0x10
#define  ADS_OP_SDATAC  0x11
#define  ADS_OP_WAKEUP  0x02
#define  ADS_OP_STANDBY 0x04
#define  ADS_OP_RESET   0x06
#define  ADS_OP_RREG   	0x20
#define  ADS_OP_WREG   	0x40
#define  ADS_OP_DEVID   0x00

union ArrayToInteger {
 byte array[4];
 int32_t integer;
};

class ARBO {
public:
	ARBO();
};

#endif