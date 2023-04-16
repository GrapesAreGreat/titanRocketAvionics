#include "bmp581.hpp"

const int BMP_SAMPLE_RATE = 10; // * 10.24 ms
// Offset the logic tick from other sensors.
int bmp_logic_ctr = BMP_SAMPLE_RATE / 5;

BMP581 pressureSensor;

// I2C address selection
//uint8_t i2cAddress = BMP581_I2C_ADDRESS_DEFAULT; // 0x47
uint8_t i2cAddress = BMP581_I2C_ADDRESS_SECONDARY; // 0x46

void bmp581_setup() {
  Wire.begin();

  if (pressureSensor.beginI2C(i2cAddress) != BMP5_OK) {
    Serial.println("Failed to connect to BMP581");
  } else {
    Serial.println("BMP581 connected!");
  }
}

bmp5_sensor_data bmp581_single_sample() {
  bmp5_sensor_data data = {0, 0};
  int8_t err = pressureSensor.getSensorData(&data);

  if (err != BMP5_OK) {
    Serial.print("Error getting data from BMP581: ");
    Serial.println(err);
  }

  return data;
}

void bmp581_log_datum(bmp5_sensor_data *data, File *file) {
    file->print("C: ");
    file->print(data->temperature);
    file->print("\t");
    file->print("Pa: ");
    file->println(data->pressure);
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

void bmp581_logic_tick(void (*on_data_func)(bmp5_sensor_data *), File *file) {
  const bmp5_sensor_data data = bmp581_single_sample();
  on_data_func(&data);
  bmp581_log_datum(&data, file);
}
