#include <SparkFun_BMP581_Arduino_Library.h>

#include "pyro_logic.hpp"
#include "imu.hpp"
#include "data_logging.hpp"
#include "buzzer.hpp"

#define TIMER0A_TCCR0A_ENABLE_BITS _BV(COM0A1)

// Define this to log data through the serial monitor.
#define PRINT_VERBOSE 1

File file;

#pragma pack(1)
struct interrupt_flags {
  bool do_pyro_tick;
  bool do_bno_tick;
  bool do_bmp_tick;
} iflags;

#pragma pack(1)
struct chute_fire_data {
  char increasing_pressure_values_count;
  bool vertical_acceleration_is_downward;
  bool vertical_acceleration_has_read_at_least_270;
  bool pressure_greater_than_72428dot50;
  bool pressure_greater_than_100959dot37;
  bool pressure_lower_than_26436dot76;
  double last_pressure;
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
  Serial.println("S0A");
  TCCR0A |= TIMER0A_TCCR0A_ENABLE_BITS;
}

void setup() {
  Serial.begin(115200);

  init_buzzer();

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

  file = SD.open(F("data_file.txt"), FILE_WRITE);
  file.println(F("start"));
  file.flush();

  // Initialize fdata.
  fdata.increasing_pressure_values_count = 0;
  fdata.last_pressure = 101325.0;

  start_timer0A();
}

void bmp5_on_data(bmp5_sensor_data *data) {
  if (fdata.last_pressure < data->pressure) {
    fdata.increasing_pressure_values_count++;
  } else {
    fdata.increasing_pressure_values_count = 0;
  }
 
  fdata.pressure_greater_than_72428dot50 = data->pressure >= 72428.50;
  fdata.pressure_greater_than_100959dot37 = data->pressure >= 100959.37;
  fdata.pressure_lower_than_26436dot76 = data->pressure <= 26436.76;
  fdata.last_pressure = data->pressure;

  #ifdef PRINT_VERBOSE
  Serial.print(F("P: "));
  Serial.print(data->pressure);
  Serial.print(F(" P greater than 72428.50: "));
  Serial.print(fdata.pressure_greater_than_72428dot50);
  Serial.print(F(" P greater than 100959.37: "));
  Serial.print(fdata.pressure_greater_than_100959dot37);
  Serial.print(F(" P less than 26436.76: "));
  Serial.print(fdata.pressure_lower_than_26436dot76);
  Serial.print(F(" Increasing P count: "));
  Serial.println((short)fdata.increasing_pressure_values_count);
  #endif
}

void bno_on_data(sensors_event_t *data) {
  // This callback only triggers on acceleration data.
  fdata.vertical_acceleration_is_downward = data->acceleration.x < -5.0; // Slightly smaller than 0 for better stability at true 0.
  fdata.vertical_acceleration_has_read_at_least_270 = (data->acceleration.x >= 270.0) & fdata.vertical_acceleration_has_read_at_least_270;

  #ifdef PRINT_VERBOSE
  Serial.print(F("Raw Vacc: x="));
  Serial.print(data->acceleration.x);
  Serial.print(" y=");
  Serial.print(data->acceleration.y);
  Serial.print(" z=");
  Serial.print(data->acceleration.z);
  Serial.print(F(" Vacc downward: "));
  Serial.print(fdata.vertical_acceleration_is_downward);
  Serial.print(F(" Vacc at least 270: "));
  Serial.println(fdata.vertical_acceleration_has_read_at_least_270);
  #endif
}

void test_if_chutes_fire() {
  // Testing if drogue chute should fire.
  const bool acceleration_increased_four_times = fdata.increasing_pressure_values_count >= 4;
  
  if (!did_drogue_fire && 
      (
        (acceleration_increased_four_times && 
         fdata.pressure_greater_than_72428dot50 && 
         fdata.vertical_acceleration_has_read_at_least_270
        ) 
      || fdata.pressure_lower_than_26436dot76
      )
     ) 
  {
    fire_drogue_signal_on();
    did_drogue_fire = true;
  }

  // Testing if main chute should fire.  
  if (!did_chute_fire &&
      did_drogue_fire &&
      fdata.vertical_acceleration_is_downward &&
      fdata.pressure_greater_than_100959dot37
     ) 
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

  if (fresh_bmp_data && fresh_bno_data) {
    test_if_chutes_fire();
    fresh_bmp_data = false;
    fresh_bno_data = false;
  }

  if (did_write_file && file) {
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
