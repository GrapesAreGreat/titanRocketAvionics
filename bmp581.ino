#include "bmp581.hpp"

#include "buzzer.hpp"

const int BMP_SAMPLE_RATE = 98; // * 10.24 ms
// Offset the logic tick from other sensors.
int bmp_logic_ctr = 2;

BMP581 pressureSensor;

// I2C address selection
//uint8_t i2cAddress = BMP581_I2C_ADDRESS_DEFAULT; // 0x47
uint8_t i2cAddress = BMP581_I2C_ADDRESS_SECONDARY; // 0x46

void bmp581_setup() {
  Wire.begin();

  while (pressureSensor.beginI2C(i2cAddress) != BMP5_OK) {
    Serial.println("BMP fail");
    pulse_buzzer(2000);
  }
    
  Serial.println("BMP success");
}

bmp5_sensor_data bmp581_single_sample() {
  bmp5_sensor_data data = {0, 0};
  int8_t err = pressureSensor.getSensorData(&data);

  if (err != BMP5_OK) {
    Serial.print("BMP err ");
    Serial.println(err);
  }

  return data;
}

void bmp581_log_datum(bmp5_sensor_data *data, File *file) {
    if (data == NULL || file == NULL) {
      return;  
    }
    
    file->print("C: ");
    file->print(data->temperature);
    file->print(" ");
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
