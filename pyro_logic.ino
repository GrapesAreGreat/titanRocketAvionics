#include "pyro_logic.hpp"

#define DROGUE_PWM_PIN 5 // PD5
#define CHUTE_PWM_PIN 6 // PD6

#define DROGUE_TCCR1A_ENABLE_BITS _BV(COM1A1)
#define CHUTE_TCCR1A_ENABLE_BITS _BV(COM1B1)

#define DROGUE_TCCR1A_ENABLE_BITS _BV(COM1A1)
#define CHUTE_TCCR1A_ENABLE_BITS _BV(COM1B1)

const int PYRO_LOGIC_SAMPLE_RATE = 10; // * 10.24 ms
// Offset sample time from other sensors.
int pyro_logic_ctr = 8;

// Counters for chute duration.
int drogue_ctr = 0;
int chute_ctr = 0;

bool drogue_signal_on = false;
bool chute_signal_on = false;

const int drogue_signal_duration = 10; // * 10.24 ms.
const int chute_signal_duration = 10; // * 10.24 ms.

void setup_drogue_and_chute_pwm() {
  pinMode(DROGUE_PWM_PIN, OUTPUT);
  pinMode(CHUTE_PWM_PIN, OUTPUT);
  // Setting the WGM register to 011 selects fast PWM mode.
  // The COM1A bits are set to 10 for clear on compare match when enabled.
  // The _BV macro sets the bit provided.
  TCCR1A = _BV(WGM21) | _BV(WGM20);
  // Setting the CS bits to 100 sets the prescalar to divide the clock by 64.
  TCCR1B = _BV(CS12);
  // The following output compare register controls the duty cycle.
  OCR1A = 180;
  OCR1B = 180;
  // Output A Frequency = 16MHz / 64 / 256 = 976.56Hz
  // Output A Duty Cycle = (180 + 1) / 256 = 70.70%
}

void pyro_logic_init() {
  setup_drogue_and_chute_pwm();
}

bool pyro_should_tick() {
  if (pyro_logic_ctr < PYRO_LOGIC_SAMPLE_RATE) {
    pyro_logic_ctr++;
    return false;
  } else {
    pyro_logic_ctr = 0;
    return true;
  }
}

void pyro_logic_tick() {

  // Logic for drogue signal.
  if (drogue_signal_on) {
    if (drogue_ctr < drogue_signal_duration) {
      drogue_ctr++;
    } else {
      drogue_ctr = 0;
      fire_drogue_signal_off();
    }
  }

  // Logic for chute signal.
  if (chute_signal_on) {
    if (chute_ctr < chute_signal_duration) {
      chute_ctr++;
    } else {
      chute_ctr = 0;
      fire_chute_signal_off();
    }
  }
}

void fire_drogue_signal_on() {
  Serial.println("Firing drogue pyro signal");
  TCCR1A |= DROGUE_TCCR1A_ENABLE_BITS;
  drogue_signal_on = true;
}

void fire_drogue_signal_off() {
  Serial.println("Stopping drogue pyro signal");
  TCCR1A &= ~(DROGUE_TCCR1A_ENABLE_BITS);
  drogue_signal_on = false;
}

void fire_chute_signal_on() {
  Serial.println("Firing chute pyro signal");
  TCCR1A |= CHUTE_TCCR1A_ENABLE_BITS;
  chute_signal_on = true;
}

void fire_chute_signal_off() {
  Serial.println("Stopping chute pyro signal");
  TCCR1A &= ~(CHUTE_TCCR1A_ENABLE_BITS);
  chute_signal_on = false;
}
