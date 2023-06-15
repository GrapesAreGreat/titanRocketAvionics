#include "bmp581.hpp"

#include "buzzer.hpp"
#include "systick.hpp"

const int BMP_SAMPLE_RATE = 5; // * 10.24 ms
// Offset the logic tick from other sensors.
int bmp_logic_ctr = 2;

BMP581 pressureSensor;

// I2C address selection
//uint8_t i2cAddress = BMP581_I2C_ADDRESS_DEFAULT; // 0x47
uint8_t i2cAddress = BMP581_I2C_ADDRESS_SECONDARY; // 0x46

void bmp581_setup() {
  Wire.begin();

  while (pressureSensor.beginI2C(i2cAddress) != BMP5_OK) {
    Serial.println(F("BMP fail"));
    pulse_buzzer(2000);
  }
    
  Serial.println(F("BMP success"));
}

bmp5_sensor_data bmp581_single_sample() {
  bmp5_sensor_data data = {0, 0};
  int8_t err = pressureSensor.getSensorData(&data);

  if (err != BMP5_OK) {
    Serial.print(F("BMP err "));
    Serial.println(err);
  }

  return data;
}

void bmp581_log_datum(bmp5_sensor_data *data, FlashLogger *logger) {
    if (data == NULL || logger == NULL) {
      return;
    }

    char buffer[60];
    char temp_buffer[12];
    char pres_buffer[12];
    dtostrf(data->temperature, 4, 4, temp_buffer);
    dtostrf(data->pressure, 4, 4, pres_buffer);
    sprintf(buffer, "T: %u C: %s Pa: %s\n", get_systick(), temp_buffer, pres_buffer);
    logger->log_string(buffer);
}

bool bmp581_should_tick() {
  if (bmp_logic_ctr < BMP_SAMPLE_RATE) {
    bmp_logic_ctr++;
    return false;
  } else {
    bmp_logic_ctr = 0;
    return true;
  }
}

void bmp581_logic_tick(void (*on_data_func)(bmp5_sensor_data *), FlashLogger *logger) {
  bmp5_sensor_data data = bmp581_single_sample();
  on_data_func(&data);
  bmp581_log_datum(&data, logger);
}
