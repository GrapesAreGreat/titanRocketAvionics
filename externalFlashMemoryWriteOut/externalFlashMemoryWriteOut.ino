#include "data_logging.hpp"

#undef DO_EXAMPLE

#ifndef DO_EXAMPLE
FlashLogger logger;
#endif

#ifdef DO_EXAMPLE

#define CHIP_SELECT_PIN 10 // PB2
#define WRITE_PROTECT_PIN 9 // PB1
#define HOLD_PIN 8 // PB0

void zeroBuffer(char* buf, uint32_t buflen){
  for(int i = 0; i < buflen; i++){
    buf[i] = 0;
  }
}


W25N512 flash = W25N512();
char* myString = "Hello world from the W25N flash chip!";
char buf[512];

void flash_example() {
  //Make sure the buffer is zero'd
  zeroBuffer(buf, sizeof(buf));

  pinMode(WRITE_PROTECT_PIN, OUTPUT);
  pinMode(HOLD_PIN, OUTPUT);

  digitalWrite(WRITE_PROTECT_PIN, HIGH);
  digitalWrite(HOLD_PIN, HIGH);
  
  if(flash.begin(CHIP_SELECT_PIN) == 0){
    Serial.println("Flash init successful");
  } else {
    Serial.println("Flash init Failed");
  }

  flash.writeEnable();

  Serial.print("Read Status: ");
  Serial.println(flash.getStatusRegister(W25N_READ_STATUS_REG), HEX);
  Serial.print("Write Status: ");
  Serial.println(flash.getStatusRegister(W25N_WRITE_STATUS_REG), HEX);
  Serial.print("Status Status: ");
  Serial.println(flash.getStatusRegister(W25N_STAT_REG), HEX);

  //put the strig into our flash buffer
  //We cant point to the string directly as it is static
  //and the buffer will be modified with the recieved data
  (void) memcpy(buf, myString, strlen(myString) + 1);
  //erase the flash page we will be writing to
  flash.blockErase(0);

  //Transfer the data we want to program and execute the program command
  flash.loadProgData(0, buf, strlen(myString) + 1);
  flash.programExecute(0);

  //zero the buffer again just so we know that the data we print will be from the flash
  zeroBuffer(buf, sizeof(buf));
  
  //read the data back and print in it
  flash.pageDataRead(0);
  flash.read(0, buf, sizeof(buf));

  //Print the data read from the flash chip
  Serial.println(buf);
}

#endif

void setup() {
  Serial.begin(115200);

  #ifndef DO_EXAMPLE
  logger.init();
  logger.write_all_to_serial();
  #else
  flash_example();
  #endif
}

void loop() {

}
