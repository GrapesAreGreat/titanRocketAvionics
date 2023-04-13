#include "data_logging.hpp"

#include <SPI.h>

const int CHIP_SELECT_PIN = 16;

void logger_setup() {
  Serial.println("Initializing SD card");

  if (!SD.begin(CHIP_SELECT_PIN)) {
    Serial.println("Failed to find SD card.");
  } else {
    Serial.println("Successfully initialized SD card");
  }
  
}
