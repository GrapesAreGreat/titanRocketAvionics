#ifndef BMP581_HPP_
#define BMP581_HPP_

#include "data_logging.hpp"
#include "SparkFun_BMP581_Arduino_Library.h"
// https://github.com/sparkfun/SparkFun_BMP581_Arduino_Library

void bmp581_setup();

void bmp581_logic_tick(void (*on_data_func)(bmp5_sensor_data *), FlashLogger *flash);

#endif
