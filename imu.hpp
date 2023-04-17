#ifndef IMU_HPP_
#define IMU_HPP_

#include <Adafruit_BNO055.h>
#include <SD.h>
// https://github.com/adafruit/Adafruit_BNO055
#include "data_logging.hpp" // For File handle object.

void bno_logic_tick(void (*on_data_func)(sensors_event_t *), File *file);

#endif
