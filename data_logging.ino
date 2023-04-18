#include "data_logging.hpp"

#include <SPI.h>
#include "buzzer.hpp"

#define CHIP_SELECT_PIN 10 // B2

void logger_setup() {
  while (!SD.begin(CHIP_SELECT_PIN)) {
    Serial.println(F("SD fail"));
    pulse_buzzer(1000);
  }
  
  Serial.println(F("SD success"));
}
