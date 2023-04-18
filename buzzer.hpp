#ifndef BUZZER_HPP_
#define BUZZER_HPP_

#define BUZZER_PIN 7 // D7

void init_buzzer() {
    pinMode(BUZZER_PIN, OUTPUT);
}

void pulse_buzzer(const int period_div_2) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(period_div_2);
    digitalWrite(BUZZER_PIN, LOW);
    delay(period_div_2);
}

#endif