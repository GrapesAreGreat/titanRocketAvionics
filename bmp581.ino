#include "bmp581.hpp"

const int BMP_SAMPLE_RATE = 10; // * 10.24 ms

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
    Serial.print("Temperature (C): ");
    Serial.print(data->temperature);
    Serial.print("\t\t");
    Serial.print("Pressure (Pa): ");
    Serial.println(data->pressure);
}

void bmp581_logic_tick(void (*on_data_func)(bmp5_sensor_data *), File *file) {
  static int bmp_ctr = 0;
  if (bmp_ctr < BMP_SAMPLE_RATE) {
    bmp_ctr++;
  } else {
    bmp_ctr = 0;
    bmp5_sensor_data data = bmp581_single_sample();
    on_data_func(&data);
    bmp581_log_datum(&data, file);
  }
}
