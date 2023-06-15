#include "data_logging.hpp"

#include "buzzer.hpp"
#include <string.h>

#define CHIP_SELECT_PIN 10 // PB2
#define WRITE_PROTECT_PIN 9 // PB1
#define HOLD_PIN 8 // PB0

#define MAX_INTERNAL_BUFFER_LEN W25N_MAX_COLUMN

FlashLogger::FlashLogger() {
  
}

void FlashLogger::init() {
  pinMode(WRITE_PROTECT_PIN, OUTPUT);
  pinMode(HOLD_PIN, OUTPUT);

  digitalWrite(WRITE_PROTECT_PIN, HIGH);
  digitalWrite(HOLD_PIN, HIGH);
  
  while (this->flash.begin(CHIP_SELECT_PIN) != 0) {
    Serial.println(F("Flash fail"));
    pulse_buzzer(2000);
  }
  Serial.println(F("Flash ready"));

  this->currentPageAddress = 0;
  this->currentInternalBufferLength = 0;

  this->flash.writeEnable();
}

void FlashLogger::erase_all_and_reset() {
  this->flash.bulkErase();
  this->currentPageAddress = 0;
  this->currentInternalBufferLength = 0;
  this->flash.blockWIP();
}

void FlashLogger::log_string(char *s) {
  this->log_string(s, strlen(s));
}
 
void FlashLogger::log_string(char *s, const size_t s_len) {
  if (this->currentInternalBufferLength + s_len >= MAX_INTERNAL_BUFFER_LEN) {
    const uint16_t bytes_can_write = MAX_INTERNAL_BUFFER_LEN - this->currentInternalBufferLength;
    const char *s_next = s + bytes_can_write;
    const uint16_t s_next_length = s_len - bytes_can_write;

    // Write the part of the string that fits into the buffer.
    this->flash.loadRandProgData(currentInternalBufferLength, s, bytes_can_write);
    // Save the internal buffer to flash memory.
    this->flush();
    // Increment the page to the next unused page.
    currentPageAddress += 64;
    // The buffer was written, so we can write to it from 0.
    currentInternalBufferLength = 0;
    // Write the part of the string that did not fit into the previous buffer.
    this->flash.loadRandProgData(currentInternalBufferLength, s_next, s_next_length);

  } else {
    this->flash.loadRandProgData(currentInternalBufferLength, s, s_len);
    currentInternalBufferLength += s_len;
  }
}

void FlashLogger::flush() {
  this->flash.programExecute(currentPageAddress);
  this->flash.blockWIP();
}

// This is really stupid, but works with high probability in our application.
bool does_buffer_appear_unwritten(char *s) {
   return (
      ((uint8_t) s[0]) == 255 && 
      ((uint8_t) s[1]) == 255 &&
      ((uint8_t) s[2]) == 255 &&
      ((uint8_t) s[3]) == 255
     );
}

void FlashLogger::write_all_to_serial() {
  // MAX_INTERNAL_BUFFER_LEN will not fit in stack memory as is.
  char buffer[MAX_INTERNAL_BUFFER_LEN / 2 + 1];
  for (uint16_t page = 0; page < W25N512GV_MAX_PAGE; page += 64) {
    // Read the page into the buffer.
    this->flash.pageDataRead(page);
    this->flash.blockWIP();

    // Read half the page because we can't fit a full page in SRAM.
    this->flash.read(0, buffer, MAX_INTERNAL_BUFFER_LEN / 2);
    this->flash.blockWIP();
    buffer[MAX_INTERNAL_BUFFER_LEN / 2] = '\0';
    if (does_buffer_appear_unwritten(buffer)) return;
    Serial.print(buffer);

    // Read the other half of the page.
    this->flash.read(MAX_INTERNAL_BUFFER_LEN / 2, buffer, MAX_INTERNAL_BUFFER_LEN / 2);
    this->flash.blockWIP();
    buffer[MAX_INTERNAL_BUFFER_LEN / 2] = '\0';
    if (does_buffer_appear_unwritten(buffer)) return;
    Serial.print(buffer);
  }
}
