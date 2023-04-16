#include "imu.hpp"

#include <Wire.h>
#include <Adafruit_Sensor.h>

const int BNO_SAMPLE_RATE = 10; // * 10.24 ms
// Offset sample time from other sensors.
int bno_logic_ctr = BNO_SAMPLE_RATE / 2;

/* This driver uses the Adafruit unified sensor library (Adafruit_Sensor),
   which provides a common 'type' for sensor data and some helper functions.
   To use this driver you will also need to download the Adafruit_Sensor
   library and include it in your libraries folder.
   You should also assign a unique ID to this sensor for use with
   the Adafruit Sensor API so that you can identify this particular
   sensor in any data logs, etc.  To assign a unique ID, simply
   provide an appropriate value in the constructor below.

   Connections
   ===========
   Connect SCL to analog x
   Connect SDA to analog y
   Connect VDD to 3.3-5V DC
   Connect GROUND to common ground
*/

// Check I2C device address and correct line below (by default address is 0x29 or 0x28)
//                                    id, address
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);

void bno_setup() {
  if (!bno.begin()) {
    Serial.println("No BNO055 detected.");
  } else {
    Serial.println("BNO055 connected!");
  }

  uint8_t system, gyro, accel, mag = 0;
  bno.getCalibration(&system, &gyro, &accel, &mag);
  Serial.print("Calibration: Sys=");
  Serial.print(system);
  Serial.print(" Gyro=");
  Serial.print(gyro);
  Serial.print(" Accel=");
  Serial.print(accel);
  Serial.print(" Mag=");
  Serial.println(mag);

  delay(10);
}

void bno_single_sample(bno_data_t *data) {
  bno.getEvent(&data->orientationData, Adafruit_BNO055::VECTOR_EULER);
  bno.getEvent(&data->angVelocityData, Adafruit_BNO055::VECTOR_GYROSCOPE);
  bno.getEvent(&data->linearAccelData, Adafruit_BNO055::VECTOR_LINEARACCEL);
  bno.getEvent(&data->magnetometerData, Adafruit_BNO055::VECTOR_MAGNETOMETER);
  bno.getEvent(&data->accelerometerData, Adafruit_BNO055::VECTOR_ACCELEROMETER);
  bno.getEvent(&data->gravityData, Adafruit_BNO055::VECTOR_GRAVITY);
}

void print_event(sensors_event_t *event, File *file) {
  double x = -1000000, y = -1000000 , z = -1000000; // Dumb values, easy to spot problem.
  if (event->type == SENSOR_TYPE_ACCELEROMETER) {
    file->print("Accl:");
    x = event->acceleration.x;
    y = event->acceleration.y;
    z = event->acceleration.z;
  }
  else if (event->type == SENSOR_TYPE_ORIENTATION) {
    file->print("Orient:");
    x = event->orientation.x;
    y = event->orientation.y;
    z = event->orientation.z;
  }
  else if (event->type == SENSOR_TYPE_MAGNETIC_FIELD) {
    file->print("Mag:");
    x = event->magnetic.x;
    y = event->magnetic.y;
    z = event->magnetic.z;
  }
  else if (event->type == SENSOR_TYPE_GYROSCOPE) {
    file->print("Gyro:");
    x = event->gyro.x;
    y = event->gyro.y;
    z = event->gyro.z;
  }
  else if (event->type == SENSOR_TYPE_ROTATION_VECTOR) {
    file->print("Rot:");
    x = event->gyro.x;
    y = event->gyro.y;
    z = event->gyro.z;
  }
  else if (event->type == SENSOR_TYPE_LINEAR_ACCELERATION) {
    file->print("Linear:");
    x = event->acceleration.x;
    y = event->acceleration.y;
    z = event->acceleration.z;
  }
  else if (event->type == SENSOR_TYPE_GRAVITY) {
    file->print("Gravity:");
    x = event->acceleration.x;
    y = event->acceleration.y;
    z = event->acceleration.z;
  }
  else {
    file->print("Unk:");
  }

  file->print(" x= ");
  file->print(x);
  file->print(" y= ");
  file->print(y);
  file->print(" z= ");
  file->println(z);
}

void bno_log_datum(bno_data_t *data, File *file) {
  print_event(&data->orientationData, file);
  print_event(&data->angVelocityData, file);
  print_event(&data->linearAccelData, file);
  print_event(&data->magnetometerData, file);
  print_event(&data->accelerometerData, file);
  print_event(&data->gravityData, file);
}

bool bno_should_tick() {
  if (bno_logic_ctr < BNO_SAMPLE_RATE) {
    bno_logic_ctr++;
    return false;
  } else {
    bno_logic_ctr = 0;
    return true;
  }
}

void bno_logic_tick(void (*on_data_func)(bno_data_t *), File *file) {
  // Stores a single sample from the sensor.
  bno_data_t data;
  bno_single_sample(&data);
  on_data_func(&data);
  bno_log_datum(&data, file);
}
