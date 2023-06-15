#include "pyro_logic.hpp"

#include "buzzer.hpp"
#include "systick.hpp"

#define DROGUE_PWM_PIN 5 // PD5
#define CHUTE_PWM_PIN 6 // PD6

// Counters for chute duration.
int drogue_ctr = 0;
int chute_ctr = 0;

bool drogue_signal_on = false;
bool chute_signal_on = false;

const int drogue_signal_duration = 198; // * 10.24 ms.
const int chute_signal_duration = 198; // * 10.24 ms.

void setup_drogue_and_chute_pwm() {
  pinMode(DROGUE_PWM_PIN, OUTPUT);
  pinMode(CHUTE_PWM_PIN, OUTPUT);
}

void pyro_logic_init() {
  setup_drogue_and_chute_pwm();
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

void fire_drogue_signal_on(FlashLogger *logger) {
  char buffer[40];
  sprintf(buffer, "T: %d Firing drogue pyro signal\n", get_systick());
  logger->log_string(buffer);
  analogWrite(DROGUE_PWM_PIN, 77);
  drogue_signal_on = true;
}

void fire_drogue_signal_off() {
  analogWrite(DROGUE_PWM_PIN, 0);
  drogue_signal_on = false;
}

void fire_chute_signal_on(FlashLogger *logger) {
  char buffer[40];
  sprintf(buffer, "T: %ud Firing chute pyro signal\n", get_systick());
  logger->log_string(buffer);
  chute_signal_on = true;
}

void fire_chute_signal_off() {
  analogWrite(CHUTE_PWM_PIN, 0);
  chute_signal_on = false;
}
