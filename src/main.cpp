
#include <Arduino.h>
#include <ShiftRegister74HC595.h>

#include "mode_switch.h"
#include "print.h"

#define BAUD_RATE 9600 // 115200

// Choose pins that cannot be used for hardware PWM
int serialDataPin = 2;  // DS aka SER (serial data) input
int clockPin = 4;       // SHCP aka SRCLK (shift register clock) input
int latchPin = 7;       // STCP aka RCLK (register clock/latch) input

int HV_Control = 5;  // PWM brightness 980Hz on pins 5 and 6, otherwise 480Hz

// OE (Output Enable)and SRCLR (Shift Register Clear)
// OE is pulled low all the time for this code.
// SRCLR is pulled high all the time for this code.

// The template parameter <1> sets the number of daisy-chained chips.
ShiftRegister74HC595<1> sr(serialDataPin, clockPin, latchPin);

// BCD for 0, ..., 9 for the LSD, MSD.
uint8_t LSD[10] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
uint8_t MSD[10] = {0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90};


void setup() {
    Serial.begin(BAUD_RATE);
    DPRINT("boot\n");
    flush();

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(HV_Control, OUTPUT);
    pinMode(INPUT_SWITCH, INPUT);

    attachInterrupt(digitalPinToInterrupt(INPUT_SWITCH), input_switch_push, RISING);

    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(HV_Control, HIGH); // Start out bright

    for (int i = 0; i < 10; ++i) {
        uint8_t bits = MSD[i] | LSD[i];
        sr.setAll(&bits);
        delay(1000);
    }

    digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
    for (int msd = 0; msd < 10; ++msd) {
        for (int lsd = 0; lsd < 10; ++lsd) {
            uint8_t bits = MSD[msd] | LSD[lsd];
            sr.setAll(&bits);
            delay(1000);
        }
    }
}
