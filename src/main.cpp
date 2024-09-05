
#include <Arduino.h>
#include <PinChangeInterrupt.h>
#include <ShiftRegister74HC595.h>

#include "RTC.h"
#include "hv_ps.h"
#include "mode_switch.h"
#include "print.h"

#define BAUD_RATE 115200

#ifndef VERSION_1_BOARD
#define VERSION_1_BOARD 0
#endif

// Choose pins that cannot be used for hardware PWM
int serialDataPin = 8; // DS aka SER (serial data) input
int clockPin = 4;      // SHCP aka SRCLK (shift register clock) input
int latchPin = 7;      // STCP aka RCLK (register clock/latch) input

int HV_Control = 5; // PWM brightness 980Hz on pins 5 and 6, otherwise 480Hz

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

    RTC_setup();

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(HV_Control, OUTPUT);
    pinMode(INPUT_SWITCH, INPUT);

    attachInterrupt(digitalPinToInterrupt(INPUT_SWITCH), input_switch_push, RISING);

    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(HV_Control, HIGH); // Start out bright

    hv_ps_setup();

    int digit_time_ms = 50;
    int random_time_ms = 1000;
    do {
        uint8_t bits[2];
        bits[0] = MSD[random(10)] | LSD[random(10)];
        bits[1] = MSD[random(10)] | LSD[random(10)];
        sr.setAll(bits);
        delay(digit_time_ms);
        random_time_ms -= digit_time_ms;
    } while (random_time_ms > 0);

    digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
    uint8_t bits[2]; // 1 is the LSD pair, 0 the MSD pair

    delay(5);
    hv_ps_adjust();

    if (time_update_handler()) {
        bits[0] = MSD[digit_3] | LSD[digit_2];
        bits[1] = MSD[digit_1] | LSD[digit_0];
        sr.setAll(bits);
    }
}
