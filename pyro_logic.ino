#include "pyro_logic.hpp"

#define DROGUE_PWM_PIN 5 // PD5
#define CHUTE_PWM_PIN 6 // PD6

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

void fire_drogue_signal_on(File *file) {
  Serial.println(F("Firing drogue pyro signal"));
  file->println(F("Firing drogue pyro signal"));
  analogWrite(DROGUE_PWM_PIN, 77);
  drogue_signal_on = true;
}

void fire_drogue_signal_off() {
  Serial.println(F("Stopping drogue pyro signal"));
  analogWrite(DROGUE_PWM_PIN, 0);
  drogue_signal_on = false;
}

void fire_chute_signal_on(File *file) {
  Serial.println(F("Firing chute pyro signal"));
  file->println(F("Firing chute pyro signal"));
  analogWrite(CHUTE_PWM_PIN, 77);
  chute_signal_on = true;
}

void fire_chute_signal_off() {
  Serial.println(F("Stopping chute pyro signal"));
  analogWrite(CHUTE_PWM_PIN, 0);
  chute_signal_on = false;
}
