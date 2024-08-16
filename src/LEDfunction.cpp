#include "LEDfuncition.h"
#include <Arduino.h>

void setupLEDs() {
    pinMode(LED_SERIAL, OUTPUT);
    pinMode(LED_RF, OUTPUT);
}

void blinkLED(int pin, int duration) {
    digitalWrite(pin, HIGH);
    delay(duration);
    digitalWrite(pin, LOW);
}
