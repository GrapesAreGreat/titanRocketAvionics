#include "data_logging.hpp"

#include <SPI.h>

const int CHIP_SELECT_PIN = 5;

void logger_setup() {
  Serial.println("Initializing SD card");

  while (!SD.begin(CHIP_SELECT_PIN)) {
    Serial.println("Failed to find SD card.");
    delay(1000);
  }

  Serial.println("Successfully initialized SD card");
}
