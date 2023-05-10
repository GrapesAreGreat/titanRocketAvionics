#ifndef IMU_HPP_
#define IMU_HPP_

#include <Adafruit_BNO055.h>
#include <SD.h>
// https://github.com/adafruit/Adafruit_BNO055
#include "data_logging.hpp" // For File handle object.

typedef enum measure {
  M_ACCELEROMETER = 0,
  M_MAGNETOMETER = 1,
  M_GYROSCOPE = 2,
  M_EULER = 3,
  M_LINEARACCEL = 4,
  M_GRAVITY = 5
} measure_t;

void bno_logic_tick(void (*on_data_func)(sensors_event_t *, const measure_t), File *file);

#endif
