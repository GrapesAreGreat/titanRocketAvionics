#include <SparkFun_BMP581_Arduino_Library.h>

#include "pyro_logic.hpp"
#include "imu.hpp"
#include "data_logging.hpp"

#define TIMER0A_TCCR0A_ENABLE_BITS _BV(COM0A1)

const char *data_file_name = "df.txt";
static File file;

void setup_timers() {
  // TIMER0A Configuration.
  // Timer0 is an 8-bit timer, counting from 0 to 256.
  TCCR0A = 0;
  // Prescale by 256.
  TCCR0B = _BV(CS02) | _BV(CS00); // Frequency = 16 MHz / 1024 = 15625 Hz
  // Enable local interrupt to OCIE1A.
  TIMSK0 = _BV(OCIE0A);
  // Set the compare match register value.
  // 0.064 ms period * 16 = 1.024 ms * 10 = 10.24 ms.
  OCR0A = 160;
}

void start_timer0A() {
  Serial.println("Starting timer 0A");
  TCCR0A |= TIMER0A_TCCR0A_ENABLE_BITS;
}

void stop_timer0A() {
  Serial.println("Stopping timer 0A");
  TCCR0A &= ~(TIMER0A_TCCR0A_ENABLE_BITS);
}

void setup() {
  // Disable all interrupts.
  cli();

  Serial.begin(115200);
  while (!Serial) delay(10);

  logger_setup();
  pyro_logic_init();
  bno_setup();
  bmp581_setup();

  setup_timers();

  // We should probably halt logging and close the file
  // once the vehicle is on the ground.
  file = SD.open(data_file_name, FILE_WRITE);

  delay(1000);

  // Enable all interrupts.
  sei();
}

void bmp5_on_data(bmp5_sensor_data *data) {
  // Do something with the data :)
}

void bno_on_data(bno_data_t *data) {
  // Do something with the data :)
}

void loop() {}

// Timer0A compare interrupt service.
ISR(TIMER0_COMPA_vect) {
  // This interrupt fires approximately every 10.24 ms.
  pyro_logic_tick();
  bmp581_logic_tick(bmp5_on_data, &file);
  bno_logic_tick(bno_on_data, &file);
}
