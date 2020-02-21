// #include <SPI.h>
SPISettings SPI_ads(SPI_FREQ, MSBFIRST, SPI_MODE1);
SPISettings SPI_fram(SPI_FREQ, MSBFIRST, SPI_MODE0);
bool doDebug;

void arbo_init(bool ovr_doDebug) {
  pinMode(ACCEL_CS, OUTPUT);
  //  pinMode(ACCEL_INT, INPUT_PULLUP);
  pinMode(ADS_CS, OUTPUT);
  pinMode(ADS_DRDY, INPUT);
  pinMode(ADS_PWDN, OUTPUT);
  pinMode(ADS_START, OUTPUT);
  pinMode(FRAM_CS, OUTPUT);
  pinMode(FRAM_HOLD, OUTPUT);
  pinMode(GRN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(SD_CS, OUTPUT);
  pinMode(SD_DET, INPUT);

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
    Serial.println("ARBO");
  } else {
    delay(12000); // give time to reprogram
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

// ACCEL override
void accel_readInt() {
  SPI.beginTransaction(SPI_fram);
  digitalWrite(ACCEL_CS, LOW);
  byte readInt[2] = {0x99};
  SPI.transfer(&readInt, sizeof(readInt));
  digitalWrite(ACCEL_CS, HIGH);
  SPI.endTransaction();
  print_buffer(readInt, 2);
}
void accel_whoami() {
  SPI.beginTransaction(SPI_fram);
  digitalWrite(ACCEL_CS, LOW);
  byte whoami[2] = {0x80};
  SPI.transfer(&whoami, sizeof(whoami));
  digitalWrite(ACCEL_CS, HIGH);
  SPI.endTransaction();
  print_buffer(whoami, 2);
}

// still does not work :(
void enableWOM() {
  SPI.beginTransaction(SPI_fram);
  digitalWrite(ACCEL_CS, LOW);
  
  byte bankSelA[2] = {0x7F,0x00};
  SPI.transfer(&bankSelA, sizeof(bankSelA));
  delay(300);
  byte RESET[2] = {0x06,0x80};
  SPI.transfer(&RESET, sizeof(RESET));
  delay(300);
  byte pwrMgmt1[2] = {0x06,0x00};
  SPI.transfer(&pwrMgmt1, sizeof(pwrMgmt1));
  delay(300);
  byte pwrMgmt2[2] = {0x07,0x38};
  SPI.transfer(&pwrMgmt2, sizeof(pwrMgmt2));
  delay(300);
  
  byte bankSelB[2] = {0x7F,0x20};
  SPI.transfer(&bankSelB, sizeof(bankSelB));
  delay(300);
  byte accelCfg[2] = {0x14,0x09};
  SPI.transfer(&accelCfg, sizeof(accelCfg));
  delay(300);

  byte bankSelC[2] = {0x7F,0x00};
  SPI.transfer(&bankSelC, sizeof(bankSelC));
  delay(300);
  byte enWOM[2] = {0x10, 0x08};
  SPI.transfer(&enWOM, sizeof(enWOM));
  delay(300);
  
  byte bankSelD[2] = {0x7F,0x20};
  SPI.transfer(&bankSelD, sizeof(bankSelD));
  delay(300);
  byte WOMlogic[2] = {0x12, 0x03};
  SPI.transfer(&WOMlogic, sizeof(WOMlogic));
  delay(300);
  byte WOMthresh[2] = {0x13, 0x00};
  SPI.transfer(&WOMthresh, sizeof(WOMthresh));
  delay(300);

  byte bankSelE[2] = {0x7F,0x00};
  SPI.transfer(&bankSelE, sizeof(bankSelE));
  delay(300);
  byte intCfg[2] = {0x0F, 0x80}; // 0xA0 for latching int, 0x80 for 50us pulse
  SPI.transfer(&intCfg, sizeof(intCfg));
  delay(300);
  byte lpCfg[2] = {0x05, 0x10};
  SPI.transfer(&lpCfg, sizeof(lpCfg));
  delay(300);
  digitalWrite(ACCEL_CS, HIGH);
  SPI.endTransaction();
}

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
  ads_ch1 = conv24to32(myBuf[5], myBuf[4], myBuf[3], rec_ADSch1);
  ads_ch2 = conv24to32(myBuf[8], myBuf[7], myBuf[6], rec_ADSch2);
  ads_ch3 = conv24to32(myBuf[11], myBuf[10], myBuf[9], rec_ADSch3);
  ads_ch4 = conv24to32(myBuf[14], myBuf[13], myBuf[12], rec_ADSch4);

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
void ads_powerDown() {
  ads_cmd(ADS_OP_SDATAC); // stop conversion
  digitalWrite(ADS_PWDN, LOW);
  // SHUTDOWN int buffer and channels
}
void ads_powerUp() {
  ads_cmd(ADS_OP_RDATAC); // cont conversion
  digitalWrite(ADS_PWDN, HIGH);
}
// ADS END

// HELPERS
int32_t addHeader(int32_t data, byte addr) {
  int32_t proto = conv24to32(addr,0,0,0);
  return proto | data;
}
void incFileName() {
  sprintf(fileName, "%08d.arbo", fileNumber);
  fileNumber++;
}
int32_t sign32(int32_t val) {
  if (val & 0x00800000) {
    return val |= 0xFF000000;
  } else {
    return val;
  }
}
int32_t conv24to32(byte b0, byte b1, byte b2, byte b3) {
  ArrayToInteger converter;
  converter.array[0] = b0;
  converter.array[1] = b1;
  converter.array[2] = b2;
  converter.array[3] = b3; // header
  return converter.integer;
}
int32_t rmHeader(int32_t val) {
  return 0x00FFFFFF & val;
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
