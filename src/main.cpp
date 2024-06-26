
#include <Arduino.h>
#include <ShiftRegister74HC595.h>

#include "mode_switch.h"
#include "print.h"

#define BAUD_RATE 9600 // 115200

#ifndef VERSION_1_BOARD
#define VERSION_1_BOARD 0
#endif

// Choose pins that cannot be used for hardware PWM
int serialDataPin = 2;  // DS aka SER (serial data) input
int clockPin = 4;       // SHCP aka SRCLK (shift register clock) input
int latchPin = 7;       // STCP aka RCLK (register clock/latch) input

int HV_Control = 5;  // PWM brightness 980Hz on pins 5 and 6, otherwise 480Hz

// OE (Output Enable) and SRCLR (Shift Register Clear)
// OE is pulled low all the time for this code.
// SRCLR is pulled high all the time for this code.

// The template parameter <1> sets the number of daisy-chained chips.
ShiftRegister74HC595<2> sr(serialDataPin, clockPin, latchPin);

// BCD for 0, ..., 9 for the LSD, MSD.
#if VERSION_1_BOARD
uint8_t MSD[10] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
uint8_t LSD[10] = {0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90};
#else
uint8_t LSD[10] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
uint8_t MSD[10] = {0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90};
#endif

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
        uint8_t bits[2];
        bits[0] = MSD[i] | LSD[i];
        bits[1] = MSD[i] | LSD[i];
        sr.setAll(bits);
        delay(1000);
    }

    digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
    uint8_t bits[2];    // 1 is the LSD pair, 0 the MSD pair
    for (int d3 = 0; d3 < 10; ++d3) {
        for (int d2 = 0; d2 < 10; ++d2) {
            bits[0] = MSD[d3] | LSD[d2];
            for (int d1 = 0; d1 < 10; ++d1) {
                for (int d0 = 0; d0 < 10; ++d0) {
                    bits[1] = MSD[d1] | LSD[d0];
                    sr.setAll(bits);
                    delay(10);
                }
            }
        }
    }
#if 0
    for (int msd = 0; msd < 10; ++msd) {
        for (int lsd = 0; lsd < 10; ++lsd) {
            uint8_t bits = MSD[msd] | LSD[lsd];
            sr.setAll(&bits);
            delay(1000);
        }
    }
#endif
}
