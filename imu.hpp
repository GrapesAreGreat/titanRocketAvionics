#ifndef IMU_HPP_
#define IMU_HPP_

#include <Adafruit_BNO055.h>
#include <SD.h>
// https://github.com/adafruit/Adafruit_BNO055
#include "data_logging.hpp" // For File handle object.

typedef struct bno_data {
    sensors_event_t orientationData;
    sensors_event_t angVelocityData;
    sensors_event_t linearAccelData;
    sensors_event_t magnetometerData;
    sensors_event_t accelerometerData;
    sensors_event_t gravityData;
} bno_data_t;

void bno_logic_tick(void (*on_data_func)(bno_data_t *), File *file);

#endif