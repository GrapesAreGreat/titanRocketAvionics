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

struct chute_fire_data {
  char increasing_pressure_values_count;
  double pressure;
  double vertical_acceleration;
} fdata;

bool fresh_bno_data = false;
bool fresh_bmp_data = false;

bool did_drogue_fire = false;
bool did_chute_fire = false;

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

  // Initialize fdata.
  fdata.increasing_pressure_values_count = 0;
  fdata.pressure = 0.0;
  fdata.vertical_acceleration = 0.0;

  start_timer0A();
}

void bmp5_on_data(bmp5_sensor_data *data) {
  const double last_pressure = fdata.pressure;
  const double current_pressure = data->pressure;
  
  if (last_pressure < current_pressure) {
    fdata.increasing_pressure_values_count++;
  } else {
    fdata.increasing_pressure_values_count = 0;
  }
  
  fdata.pressure = data->pressure;
}

void bno_on_data(sensors_event_t *data) {
  // This callback only triggers on acceleration data.
  fdata.vertical_acceleration = data->acceleration.x;
}

void test_if_chutes_fire() {
  // Testing if drogue chute should fire.
  const bool acceleration_increased_four_times = fdata.increasing_pressure_values_count >= 4;
  
  if (!did_drogue_fire && 
      ((acceleration_increased_four_times &&
      fdata.pressure >= 72428.50 &&
      fdata.vertical_acceleration >= 270.0) || 
      fdata.pressure <= 26436.76)) 
  {
    fire_drogue_signal_on();
    did_drogue_fire = true;
  }

  // Testing if main chute should fire.
  const bool accel_is_downward = fdata.vertical_acceleration < 0.0;
  
  if (!did_chute_fire &&
      accel_is_downward &&
      fdata.pressure >= 100959.37) 
  {
    fire_chute_signal_on();
    did_chute_fire = true;
  }
}

void loop() {
  bool did_write_file = false;

  if (iflags.do_pyro_tick) {
    pyro_logic_tick();
    iflags.do_pyro_tick = false;
  }

  if (iflags.do_bmp_tick) {
    bmp581_logic_tick(bmp5_on_data, &file);
    did_write_file = true;
    fresh_bmp_data = true;
    iflags.do_bmp_tick = false;
  }

  if (iflags.do_bno_tick) {
    bno_logic_tick(bno_on_data, &file);
    did_write_file = true;
    fresh_bno_data = true;
    iflags.do_bno_tick = false;
  }

  if (did_write_file) {
    file.flush();
  }

  if (fresh_bmp_data && fresh_bno_data) {
    test_if_chutes_fire();
    fresh_bmp_data = false;
    fresh_bno_data = false;
  }
}

// Timer0A compare interrupt service.
ISR(TIMER0_COMPA_vect) {
  // This interrupt fires approximately every 10.24 ms.
  iflags.do_pyro_tick = pyro_should_tick();
  iflags.do_bmp_tick = bmp581_should_tick();
  iflags.do_bno_tick = bno_should_tick();
}
