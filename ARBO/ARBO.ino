// #include <SPI.h>
SPISettings SPI_ads(SPI_FREQ, MSBFIRST, SPI_MODE1);
SPISettings SPI_fram(SPI_FREQ, MSBFIRST, SPI_MODE0);
bool doDebug = false;

void arbo_init(bool ovr_doDebug) {
  pinMode(ACCEL_CS, OUTPUT);
  pinMode(ACCEL_INT, INPUT_PULLUP);
  pinMode(ADS_CS, OUTPUT);
  pinMode(ADS_DRDY, INPUT);
  pinMode(ADS_PWDN, OUTPUT);
  pinMode(ADS_START, OUTPUT);
  pinMode(FRAM_CS, OUTPUT);
  pinMode(FRAM_HOLD, OUTPUT);
  pinMode(GRN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(SD_CS, OUTPUT);

  digitalWrite(ACCEL_CS, HIGH);
  digitalWrite(ADS_CS, HIGH);
  digitalWrite(ADS_PWDN, HIGH);
  digitalWrite(ADS_START, HIGH);
  digitalWrite(FRAM_CS, HIGH);
  digitalWrite(FRAM_HOLD, HIGH);
  digitalWrite(GRN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(SD_CS, HIGH);

  SPI.begin();
  doDebug = ovr_doDebug; // overwrite define?
  if (doDebug) {
    Serial.begin(9600);
    while (!Serial) {};
//    if (!sd.begin(SD_CS, SD_SCK_MHZ(1))) { // replace with SPI_fram
//      Serial.println("SD ERROR");
//    }
    Serial.println("ARBO initialized...");
  } if (Serial.available() > 0) {

  }
}

// hard-coding pin13 for compatibility with mini
void arbo_blink(int postdelay) {
  for (int i = 0; i < 3; i++) {
    digitalWrite(13, HIGH);
    delay(50);
    digitalWrite(13, LOW);
    delay(50);
  }
  delay(postdelay);
}

//NOT WORKING?
// void arbo_debug(char *printStr) {
//   if (doDebug) {
//     if (newline) {
//       Serial.println(printStr);
//     } else {
//       Serial.print(printStr);
//     }
//   }
// }

// FRAM START //
void fram_on() {
  SPI.beginTransaction(SPI_fram);
  digitalWrite(FRAM_CS, LOW);
}
void fram_off() {
  digitalWrite(FRAM_CS, HIGH);
  SPI.endTransaction();
}
void fram_sleep() {
  byte myBuf[1] = {FRAM_OP_SLEEP};
  fram_on();
  SPI.transfer(&myBuf, sizeof(myBuf));
  fram_off();
}
void fram_wake() {
  // just toggle CS to wake
  digitalWrite(FRAM_CS, LOW);
  digitalWrite(FRAM_CS, HIGH);
}
void fram_holdOn() {
  digitalWrite(FRAM_HOLD, LOW);
}
void fram_holdOff() {
  digitalWrite(FRAM_HOLD, HIGH);
}
void fram_writeEnable() {
  byte myBuf[1] = {FRAM_OP_WREN};
  fram_on();
  SPI.transfer(&myBuf, sizeof(myBuf));
  fram_off();
}
void fram_writeDisable() {
  byte myBuf[1] = {FRAM_OP_WRDI};
  fram_on();
  SPI.transfer(&myBuf, sizeof(myBuf));
  fram_off();
}
bool fram_deviceId() {
  byte myBuf[5] = {FRAM_OP_RDID};
  byte mfgId = 0x04;
  byte contCode = 0x7F;
  byte prodId1 = 0x27;
  byte prodId2 = 0x03;
  fram_on();
  SPI.transfer(&myBuf, sizeof(myBuf));
  fram_off();
  // myBuf[0] is for the op-code
  if (myBuf[1] == mfgId && myBuf[2] == contCode && myBuf[3] == prodId1 && myBuf[4] == prodId2) {
    return true;
  } else {
    return false;
  }
}
void fram_writeInt(int32_t memAddr, int32_t data) {
  byte myBuf[8] = {FRAM_OP_WRITE, memAddr >> 16, memAddr >> 8, memAddr, data >> 24, data >> 16, data >> 8, data};
  fram_writeEnable();
  fram_on();
  SPI.transfer(&myBuf, sizeof(myBuf));
  fram_off();
  fram_writeDisable();
}
void fram_writeByte(int32_t memAddr, byte data) {
  byte myBuf[5] = {FRAM_OP_WRITE, memAddr >> 16, memAddr >> 8, memAddr, data};
  fram_writeEnable();
  fram_on();
  SPI.transfer(&myBuf, sizeof(myBuf));
  fram_off();
  fram_writeDisable();
}
void fram_readChunk(uint32_t memAddr, byte *values, size_t sz) {
  byte addrBuf[4] = {FRAM_OP_READ, memAddr >> 16, memAddr >> 8, memAddr};
  fram_on();
  SPI.transfer(&addrBuf, sizeof(addrBuf));
  for (size_t i = 0; i < sz; i++) {
    byte x = SPI.transfer(0);
    values[i] = x;
  }
  fram_off();
}
// technically could read larger data sizes
int fram_readInt(int32_t memAddr) {
  // op-code, addr, addr, addr, dummy 8-cycles, read 4 bytes (int)
  byte myBuf[8] = {FRAM_OP_READ, memAddr >> 16, memAddr >> 8, memAddr};
  fram_on();
  SPI.transfer(&myBuf, sizeof(myBuf));
  fram_off();
  ArrayToInteger converter;
  converter.array[0] = myBuf[7];
  converter.array[1] = myBuf[6];
  converter.array[2] = myBuf[5];
  converter.array[3] = myBuf[4];
  return converter.integer;
}
byte fram_readByte(int32_t memAddr) {
  // op-code, addr, addr, addr, empty for read data
  byte myBuf[5] = {FRAM_OP_READ, memAddr >> 16, memAddr >> 8, memAddr};
  fram_on();
  SPI.transfer(&myBuf, sizeof(myBuf));
  fram_off();
  return myBuf[4];
}
int32_t int32Addr(int32_t chunkId) {
  return 4 * (chunkId);
}
// FRAM END //

// ADS START
void ads_wreg(byte rrrrr, byte data) {
  ads_on();
  byte myBuf[3] = {ADS_OP_WREG | rrrrr, 0x00, data}; // op-code then 0x00 to write 1 register
  SPI.transfer(&myBuf, sizeof(myBuf));
  ads_off();
}

void ads_read() {
  size_t bufSz = (24 + (ADSchs * 24)) / 8;
  byte myBuf[bufSz] = {};
  while (digitalRead(ADS_DRDY)) {} // wait for _DRDY to go low
  ads_on();
  SPI.transfer(&myBuf, sizeof(myBuf));
  ads_off();
  ads_ch1 = sign24to32(myBuf[3], myBuf[4], myBuf[5]);
  ads_ch2 = sign24to32(myBuf[6], myBuf[7], myBuf[8]);
  ads_ch3 = sign24to32(myBuf[9], myBuf[10], myBuf[11]);
  ads_ch4 = sign24to32(myBuf[12], myBuf[13], myBuf[14]);

  //  int i;
  //  print_buffer(myBuf,sizeof(myBuf));
  //  for (i = 1; i <= 1; i++) {
  //    int bufStart = (i * 3);
  //    if (i == 1) {
  //      Serial.println(sign24to32(myBuf[bufStart], myBuf[bufStart + 1], myBuf[bufStart + 2]));
  //    } else {
  //      Serial.print(sign24to32(myBuf[bufStart], myBuf[bufStart + 1], myBuf[bufStart + 2]));
  //      Serial.print("\t");
  //    }
  //}

  //  ArrayToInteger converter;
  //  converter.array[0] = 0x00;
  //  converter.array[1] = myBuf[3];
  //  converter.array[2] = myBuf[4];
  //  converter.array[3] = myBuf[5];
  //  return converter.integer;
}

void ads_startConv() { // could replace with START cmd
  digitalWrite(ADS_START, HIGH);
}
void ads_endConv() {
  digitalWrite(ADS_START, LOW);
}

void ads_cmd(byte cmd) {
  ads_on();
  SPI.transfer(&cmd, 1);
  ads_off();
}

bool ads_deviceId() {
  ads_on();
  byte myBuf[3] = {ADS_OP_SDATAC, ADS_OP_RREG | ADS_OP_DEVID, 0x00}; // stop data continuous, read 1 reg
  SPI.transfer(&myBuf, sizeof(myBuf));
  ads_off();
  if (myBuf[0] == 0x90 || myBuf[0] == 0x91 || myBuf[0] == 0x92) { // ADS129x
    return true;
  } else {
    Serial.println("deviceId buffer:");
    print_buffer(myBuf, sizeof(myBuf));
    return false;
  }
}

void ads_on() {
  SPI.beginTransaction(SPI_ads);
  digitalWrite(ADS_CS, LOW);
}
void ads_off() {
  digitalWrite(ADS_CS, HIGH);
  SPI.endTransaction();
}
// ADS END

// HELPERS
int32_t sign24to32(byte b1, byte b2, byte b3) {
  ArrayToInteger converter;
  converter.array[0] = b3;
  converter.array[1] = b2;
  converter.array[2] = b1;
  converter.array[3] = 0;
  // return converter.integer;
  if (converter.integer & 0x00800000) {
    return converter.integer |= 0xFF000000;
  } else {
    return converter.integer;
  }
}

void print_buffer(byte arry[], int sz) {
  for (int i = 0; i < sz; i++) {
    if (i == sz - 1) {
      Serial.println(arry[i], DEC);
    } else {
      Serial.print(arry[i], DEC);
      Serial.print("\t");
    }
  }
}
float measureBatt() {
  pinMode(FRAM_HOLD, INPUT);
  float measuredvbat = analogRead(FRAM_HOLD);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
  pinMode(FRAM_HOLD, OUTPUT);
  return measuredvbat;
}

float average (int32_t * array, int len) {
  long sum = 0L ;  // sum will be larger than an item, long for safety.
  for (int i = 0 ; i < len ; i++) {
    sum += array [i] ;
  }
  return  ((float) sum) / len ;  // average will be fractional, so float may be appropriate.
}

void printFormattedFloat(float val, uint8_t leading, uint8_t decimals) {
  float aval = abs(val);
  if (val < 0) {
    Serial.print("-");
  } else {
    Serial.print(" ");
  }
  for ( uint8_t indi = 0; indi < leading; indi++ ) {
    uint32_t tenpow = 0;
    if ( indi < (leading - 1) ) {
      tenpow = 1;
    }
    for (uint8_t c = 0; c < (leading - 1 - indi); c++) {
      tenpow *= 10;
    }
    if ( aval < tenpow) {
      Serial.print("0");
    } else {
      break;
    }
  }
  if (val < 0) {
    Serial.print(-val, decimals);
  } else {
    Serial.print(val, decimals);
  }
}

void printScaledAGMT( ICM_20948_AGMT_t agmt) {
  Serial.print("Scaled. Acc (mg) [ ");
  printFormattedFloat( myICM.accX(), 5, 2 );
  Serial.print(", ");
  printFormattedFloat( myICM.accY(), 5, 2 );
  Serial.print(", ");
  printFormattedFloat( myICM.accZ(), 5, 2 );
  Serial.print(" ], Gyr (DPS) [ ");
  printFormattedFloat( myICM.gyrX(), 5, 2 );
  Serial.print(", ");
  printFormattedFloat( myICM.gyrY(), 5, 2 );
  Serial.print(", ");
  printFormattedFloat( myICM.gyrZ(), 5, 2 );
  Serial.print(" ], Mag (uT) [ ");
  printFormattedFloat( myICM.magX(), 5, 2 );
  Serial.print(", ");
  printFormattedFloat( myICM.magY(), 5, 2 );
  Serial.print(", ");
  printFormattedFloat( myICM.magZ(), 5, 2 );
  Serial.print(" ], Tmp (C) [ ");
  printFormattedFloat( myICM.temp(), 5, 2 );
  Serial.print(" ]");
  Serial.println();
}
