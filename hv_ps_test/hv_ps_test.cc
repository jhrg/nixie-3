/**
 * Test the High Voltage Power Supply.
 */

#include "hv_ps.h"

#include <Arduino.h>

#include "print.h"

#define BAUD_RATE 115200

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    Serial.begin(BAUD_RATE);
    DPRINT("boot\n");
    flush();

    hv_ps_setup();

    void hv_ps_adjust();

    digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
    hv_ps_adjust();
}
