#include "W25N512GVEIG.h"

#include <SPI.h>

W25N512::W25N512() {}

void W25N512::sendData(char *buf, const unsigned int len) {
  SPI.beginTransaction(SPISettings(80000000, MSBFIRST, SPI_MODE0));
  digitalWrite(this->_cs, LOW);
  SPI.transfer(buf, len);
  digitalWrite(this->_cs, HIGH);
  SPI.endTransaction();
}

int W25N512::begin(unsigned int cs) {
    SPI.begin();
    this->_cs = cs;
    pinMode(this->_cs, OUTPUT);
    digitalWrite(this->_cs, HIGH);

    this->reset();

    uint8_t jedec[5] = {W25N_JEDEC_ID, 0x00, 0x00, 0x00, 0x00};
    this->sendData(jedec, 5);
    if (jedec[2] == WINBOND_MAN_ID) {
      const uint16_t device_id = ((uint16_t) jedec[3]) << 8 | jedec[4];
      if (device_id == W25N512GV_DEV_ID) {
        this->setStatusRegister(W25N_PROT_REG, 0x00);
        return 0;
      }
    }

    return 1;
}

void W25N512::reset() {
  char buf[] = {W25N_RESET};
  this->sendData(buf, sizeof(buf));
  delayMicroseconds(600);
 }

uint8_t W25N512::getStatusRegister(const char reg) {
  uint8_t buff[3] = {W25N_READ_STATUS_REG, reg, 0x00};
  this->sendData(buff, sizeof(buff));
  return buff[2];
}

uint8_t W25N512::setStatusRegister(const char reg, const char set) {
  uint8_t buff[3] = {W25N_WRITE_STATUS_REG, reg, set};
  this->sendData(buff, sizeof(buff));
}

uint16_t W25N512::getMaxPage() {
  return W25N512GV_MAX_PAGE;
}

void W25N512::writeEnable() {
  char buf[] = {W25N_WRITE_ENABLE};
  this->sendData(buf, sizeof(buf));
}

void W25N512::writeDisable() {
  char buf[] = {W25N_WRITE_DISABLE};
  this->sendData(buf, sizeof(buf));
}

int W25N512::blockErase(uint16_t pageAdd) {
  if(pageAdd > this->getMaxPage()) return 1;
  char pageHigh = (char)((pageAdd & 0xFF00) >> 8);
  char pageLow = (char)(pageAdd);
  char buf[4] = {W25N_BLOCK_ERASE, 0x00, pageHigh, pageLow};
  this->blockWIP();
  this->writeEnable();
  this->sendData(buf, sizeof(buf));
  return 0;
}

int W25N512::bulkErase() {
  int error = 0;
  for(uint16_t i = 0; i < this->getMaxPage(); i += 64){
    if((error = this->blockErase(i)) != 0) return error;
  }
  return 0;
}

int W25N512::loadProgData(uint16_t columnAdd, char* buf, uint16_t dataLen) {
  if(columnAdd > (uint32_t)W25N_MAX_COLUMN) return 1;
  if(dataLen > (uint32_t)W25N_MAX_COLUMN - columnAdd) return 1;
  char columnHigh = (columnAdd & 0xFF00) >> 8;
  char columnLow = columnAdd & 0xff;
  char cmdbuf[3] = {W25N_PROG_DATA_LOAD, columnHigh, columnLow};
  this->blockWIP();
  this->writeEnable();
  SPI.beginTransaction(SPISettings(80000000, MSBFIRST, SPI_MODE0));
  digitalWrite(_cs, LOW);
  SPI.transfer(cmdbuf, sizeof(cmdbuf));
  SPI.transfer(buf, dataLen);
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
  return 0;
}

int W25N512::loadRandProgData(uint16_t columnAdd, char* buf, uint16_t dataLen) {
  if(columnAdd > (uint16_t)W25N_MAX_COLUMN) return 1;
  if(dataLen > (uint16_t)W25N_MAX_COLUMN - columnAdd) return 1;
  char columnHigh = (columnAdd & 0xFF00) >> 8;
  char columnLow = columnAdd & 0xff;
  char cmdbuf[3] = {W25N_RAND_PROG_DATA_LOAD, columnHigh, columnLow};
  this->blockWIP();
  this->writeEnable();
  SPI.beginTransaction(SPISettings(80000000, MSBFIRST, SPI_MODE0));
  digitalWrite(_cs, LOW);
  SPI.transfer(cmdbuf, sizeof(cmdbuf));
  SPI.transfer(buf, dataLen);
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
  return 0;
}

int W25N512::programExecute(uint16_t pageAdd) {
  if(pageAdd > this->getMaxPage()) return 1;
  char pageHigh = (char)((pageAdd & 0xFF00) >> 8);
  char pageLow = (char)(pageAdd);
  this->writeEnable();
  char buf[4] = {W25N_PROG_EXECUTE, 0x00, pageHigh, pageLow};
  this->sendData(buf, sizeof(buf));
  return 0;
}

int W25N512::pageDataRead(uint16_t pageAdd) {
  if(pageAdd > this->getMaxPage()) return 1;
  char pageHigh = (char)((pageAdd & 0xFF00) >> 8);
  char pageLow = (char)(pageAdd);
  char buf[4] = {W25N_PAGE_DATA_READ, 0x00, pageHigh, pageLow};
  this->blockWIP();
  this->sendData(buf, sizeof(buf));
  return 0;
}

int W25N512::read(uint16_t columnAdd, char* buf, uint16_t dataLen) {
  if(columnAdd > (uint16_t)W25N_MAX_COLUMN) return 1;
  if(dataLen > (uint16_t)W25N_MAX_COLUMN - columnAdd) return 1;
  char columnHigh = (columnAdd & 0xFF00) >> 8;
  char columnLow = columnAdd & 0xff;
  char cmdbuf[4] = {W25N_READ, columnHigh, columnLow, 0x00};
  this->blockWIP();
  SPI.beginTransaction(SPISettings(80000000, MSBFIRST, SPI_MODE0));
  digitalWrite(_cs, LOW);
  SPI.transfer(cmdbuf, sizeof(cmdbuf));
  SPI.transfer(buf, dataLen);
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
  return 0;
}

//Returns the Write In Progress bit from flash.
int W25N512::checkWIP() {
  char status = this->getStatusRegister(W25N_STAT_REG);
  if(status & 0x01){
    return 1;
  }
  return 0;
}                   

int W25N512::blockWIP() {
  //Max WIP time is 10ms for block erase so 15 should be a max.
  unsigned long tstamp = millis();
  while(this->checkWIP()){
    delay(1);
    if (millis() > tstamp + 15) return 1;
  }
  return 0;
}

int W25N512::checkStatus() {
  return(this->getStatusRegister(W25N_STAT_REG));
}
