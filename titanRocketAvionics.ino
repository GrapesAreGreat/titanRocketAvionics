#include <SparkFun_BMP581_Arduino_Library.h>

#include "pyro_logic.hpp"
#include "imu.hpp"
#include "data_logging.hpp"

#define TIMER0A_TCCR0A_ENABLE_BITS _BV(COM0A1)

const char *data_file_name = "df.txt";
const char *start_of_logging_section_str = "start";
File file;

struct interrupt_flags {
  bool do_pyro_tick;
  bool do_bno_tick;
  bool do_bmp_tick;
} iflags;

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
  Serial.begin(115200);
  while (!Serial) delay(10);

  logger_setup();
  pyro_logic_init();
  bmp581_setup();
  bno_setup();

  // Disable all interrupts.
  cli();

  setup_timers();

  delay(10);

  // Enable all interrupts.
  sei();

  file = SD.open(data_file_name, FILE_WRITE);
  file.println(start_of_logging_section_str);
  file.flush();

  start_timer0A();
}

void bmp5_on_data(bmp5_sensor_data *data) {
  // Do something with the data :)
}

void bno_on_data(bno_data_t *data) {
  // Do something with the data :)
}

void loop() {
  bool did_write_file = false;

  if (iflags.do_pyro_tick) {
    pyro_logic_tick();
  }

  if (iflags.do_bmp_tick) {
    bmp581_logic_tick(bmp5_on_data, &file);
    did_write_file = true;
  }

  if (iflags.do_bno_tick) {
    bno_logic_tick(bno_on_data, &file);
    did_write_file = true;
  }

  if (did_write_file) {
    file.flush();
  }
}

// Timer0A compare interrupt service.
ISR(TIMER0_COMPA_vect) {
  // This interrupt fires approximately every 10.24 ms.
  iflags.do_pyro_tick = pyro_should_tick();
  iflags.do_bmp_tick = bmp581_should_tick();
  iflags.do_bno_tick = bno_should_tick();
}
