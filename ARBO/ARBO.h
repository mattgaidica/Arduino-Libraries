#ifndef ARBO_H
#define ARBO_H

#include "Arduino.h"

#define SPI_FREQ 1000000

// FRAM op-codes
#define WREN   	0x06 //0b00000110
#define WRDI    0x04 //0b00000100
#define RDSR    0x05 //0b00000101
#define WRSR    0x01 //0b00000001
#define READ    0x03 //0b00000011
#define WRITE   0x02 //0b00000010
#define RDID    0x9F //0b10011111
#define FSTRD   0x0B //0b00001011
#define SLEEP   0xB9 //0b10111001
// uint32_t maxMem   0x1FFFF;

// ADS129x op-codes
#define RDATAC	0x10
#define SDATAC  0x11
#define WAKEUP  0x02
#define STANDBY 0x04
#define RESET   0x06
#define RREG   	0x20
#define WREG   	0x40
#define DEVID   0x00

union ArrayToInteger {
 byte array[4];
 int32_t integer;
};


class ARBO {
public:
	ARBO();
};

#endif