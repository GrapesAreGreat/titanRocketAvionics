#include <SparkFun_BMP581_Arduino_Library.h>

#include "pyro_logic.hpp"
#include "imu.hpp"
#include "data_logging.hpp"
#include "buzzer.hpp"
#include "systick.hpp"

// This entire system could be converted into an event driven state machine.
// Future maintainers should consider this possibility. 

#define TIMER2A_TCCR2A_ENABLE_BITS _BV(COM2A1)

// 69980.62 PA (9889.63ft) - 70000 PA (9882.37ft) = -19.38 PA
#define PRESSURE_DELTA (-19.38)

// Define this to log data through the serial monitor.
#define PRINT_VERBOSE 1

// Mojave new area elevation is 740 m (2428ft) ~ 92744.77
// Addition of 1000ft for elevation of 3428ft ~ 89386.67
#define PA_ABOVE_1000_FT_PLUS_GROUND_LEVEL 89386.67

FlashLogger logger;

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
  bool pressure_greater_than_ground_plus_1000_ft;
  double last_pressure;
} fdata;

bool fresh_bno_data = false;
bool fresh_bmp_data = false;

bool did_drogue_fire = false;
bool did_chute_fire = false;

const uint8_t flash_flush_rate = 97; // * 10.24 ms
uint8_t flash_flush_counter = 0;

void setup_timers() {
  // TIMER2A Configuration.
  // Timer2 is an 8-bit timer, counting from 0 to 256.
  TCCR2A = 0;
  // Prescale by 1024.
  TCCR2B = _BV(CS22) | _BV(CS21) | _BV(CS20); // Frequency = 16 MHz / 1024 = 15625 Hz
  // Enable local interrupt to OCIE2A.
  TIMSK2 = _BV(OCIE2A);
  // Set the compare match register value.
  // 0.064 ms period * 16 = 1.024 ms * 10 = 10.24 ms.
  OCR2A = 160;
}

void start_timer0A() {
  Serial.println(F("S2A"));
  TCCR2A |= TIMER2A_TCCR2A_ENABLE_BITS;
}

void setup() {
  Serial.begin(115200);

  init_buzzer();

  logger.init();
  pyro_logic_init();
  bmp581_setup();
  bno_setup();

  // Wait ten seconds so data is not eliminated immediately.
  pulse_buzzer(1000);
  delay(10000);
  pulse_buzzer(1000);

  // Clear the flash memory. Takes about 30 seconds.
  logger.erase_all_and_reset();
  delay(30000);
  pulse_buzzer(1000);

  setup_timers();

  // Initialize fdata.
  fdata.increasing_pressure_values_count = 0;
  fdata.last_pressure = 101325.0;

  start_timer0A();
}

void bmp5_on_data(bmp5_sensor_data *data) {
  if (fdata.last_pressure - data->pressure < PRESSURE_DELTA) {
    fdata.increasing_pressure_values_count++;
  } else {
    fdata.increasing_pressure_values_count = 0;
  }
 
  fdata.pressure_greater_than_ground_plus_1000_ft = data->pressure >= PA_ABOVE_1000_FT_PLUS_GROUND_LEVEL;
  fdata.last_pressure = data->pressure;

  #ifdef PRINT_VERBOSE
  Serial.print(F("P: "));
  Serial.print(data->pressure);
  Serial.print(F(" P greater than 72428.50: "));
  Serial.print(F(" P greater than 100959.37: "));
  Serial.print(fdata.pressure_greater_than_ground_plus_1000_ft);
  Serial.print(F(" P less than 26436.76: "));
  Serial.print(F(" Increasing P count: "));
  Serial.println((short)fdata.increasing_pressure_values_count);
  #endif
}

void bno_on_data(sensors_event_t *data, const measure_t data_type) {
  switch (data_type) {

    case M_ACCELEROMETER:
      // Slightly smaller than 0 for better stability at true 0.
      fdata.vertical_acceleration_is_downward = data->acceleration.x < -1.0; 

      #ifdef PRINT_VERBOSE
      Serial.print(F("Raw Vacc: x="));
      Serial.print(data->acceleration.x);
      Serial.print(" y=");
      Serial.print(data->acceleration.y);
      Serial.print(" z=");
      Serial.print(data->acceleration.z);
      Serial.print(F(" Vacc downward: "));
      Serial.println(fdata.vertical_acceleration_is_downward);
      #endif

      break;
    default:
      break;
  }

}

void test_if_chutes_fire() {
  // Testing if drogue chute should fire.
  const bool pressure_increased_eight_times = fdata.increasing_pressure_values_count >= 8;
  
  if (!did_drogue_fire && pressure_increased_eight_times) 
  {
    fire_drogue_signal_on(&logger);
    did_drogue_fire = true;
  }

  // Testing if main chute should fire.  
  if (!did_chute_fire &&
      did_drogue_fire &&
      fdata.pressure_greater_than_ground_plus_1000_ft
     ) 
  {
    fire_chute_signal_on(&logger);
    did_chute_fire = true;
  }
}

void loop() {
  if (iflags.do_pyro_tick) {
    pyro_logic_tick();
    iflags.do_pyro_tick = false;
  }

  if (iflags.do_bmp_tick) {
    bmp581_logic_tick(bmp5_on_data, &logger);
    fresh_bmp_data = true;
    iflags.do_bmp_tick = false;
  }

  if (iflags.do_bno_tick) {
    bno_logic_tick(bno_on_data, &logger);
    fresh_bno_data = true;
    iflags.do_bno_tick = false;
  }

  if (fresh_bmp_data && fresh_bno_data) {
    test_if_chutes_fire();
    fresh_bmp_data = false;
    fresh_bno_data = false;
  }
}

// Timer2A compare interrupt service.
ISR(TIMER2_COMPA_vect) {
  // This interrupt fires approximately every 10.24 ms.
  iflags.do_pyro_tick = true;
  iflags.do_bmp_tick = bmp581_should_tick();
  iflags.do_bno_tick = bno_should_tick();
  tick_systick();
}
