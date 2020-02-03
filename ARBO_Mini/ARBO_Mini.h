#ifndef ARBO_MINI_H
#define ARBO_MINI_H

#define RED_LED 	20 // was 13
#define GRN_LED 	8 // placeholder, 8 is NC, no GRN_LED on mini
#define ADS_CS 		38
#define ADS_DRDY 	12
#define ADS_PWDN 	10
#define ADS_START 	11
#define FRAM_CS		13 // rewired to RED_LED
#define FRAM_HOLD 	9 // rewired to RD_VBATT
#define SD_CS		5
// #define SD_DET 		7 // no detent on Mini
#define ACCEL_CS 	1
#define ACCEL_INT 	0 // analog pin 4!

#endif