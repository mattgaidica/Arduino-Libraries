SPISettings SPI_ads(SPI_FREQ, MSBFIRST, SPI_MODE1);
SPISettings SPI_fram(SPI_FREQ, MSBFIRST, SPI_MODE0);

void init() {
  pinMode(ACCEL_CS, OUTPUT);
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
}

void fram_on() {
  SPI.beginTransaction(SPI_fram);
  digitalWrite(FRAM_CS, LOW);
}
void fram_off() {
  digitalWrite(FRAM_CS, HIGH);
  SPI.endTransaction();
}
void fram_sleep() {
  byte myBuf[1] = {SLEEP};
  fram_on();
  SPI.transfer(&myBuf, sizeof(myBuf));
  fram_off();
}
void fram_wake() {
  // just toggle CS to wake
  digitalWrite(FRAM_CS, LOW);
  digitalWrite(FRAM_CS, HIGH);
}
void fram_writeEnable() {
  byte myBuf[1] = {WREN};
  fram_on();
  SPI.transfer(&myBuf, sizeof(myBuf));
  fram_off();
}
void fram_writeDisable() {
  byte myBuf[1] = {WRDI};
  fram_on();
  SPI.transfer(&myBuf, sizeof(myBuf));
  fram_off();
}
bool fram_deviceId() {
  byte myBuf[5] = {RDID};
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
int32_t int32Addr(int32_t thisInt) {
  return 4 * (thisInt);
}

void fram_writeInt(int32_t memAddr, int32_t data) {
  byte myBuf[8] = {WRITE, memAddr >> 16, memAddr >> 8, memAddr, data >> 24, data >> 16, data >> 8, data};
  fram_writeEnable();
  fram_on();
  SPI.transfer(&myBuf, sizeof(myBuf));
  fram_off();
  fram_writeDisable();
}
void fram_writeByte(int32_t memAddr, byte data) {
  byte myBuf[5] = {WRITE, memAddr >> 16, memAddr >> 8, memAddr, data};
  fram_writeEnable();
  fram_on();
  SPI.transfer(&myBuf, sizeof(myBuf));
  fram_off();
  fram_writeDisable();
}

byte *fram_readChunk(int32_t memAddr) {
  // op-code, addr, addr, addr, dummy 8-cycles, read 4 bytes (int)
  //  uBuf[0] = READ;
  //  uBuf[1] = memAddr >> 16;
  //  uBuf[2] = memAddr >> 8;
  //  uBuf[3] = memAddr;
  byte myBuf[4] = {READ, memAddr >> 16, memAddr >> 8, memAddr};
  fram_on();
  SPI.transfer(&myBuf, sizeof(myBuf));
  SPI.transfer(&uBuf, sizeof(uBuf));
  fram_off();
}

// technically could read larger data sizes
int fram_readInt(int32_t memAddr) {
  // op-code, addr, addr, addr, dummy 8-cycles, read 4 bytes (int)
  byte myBuf[8] = {READ, memAddr >> 16, memAddr >> 8, memAddr};
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
  byte myBuf[5] = {READ, memAddr >> 16, memAddr >> 8, memAddr};
  fram_on();
  SPI.transfer(&myBuf, sizeof(myBuf));
  fram_off();
  return myBuf[4];
}