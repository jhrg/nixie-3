
#include <Arduino.h>
#include <PinChangeInterrupt.h>

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

// OE (Output Enable) and SRCLR (Shift Register Clear)
// OE is pulled low all the time for this code.
// SRCLR is pulled high all the time for this code.

int HV_Control = 5; // PWM brightness 980Hz on pins 5 and 6, otherwise 480Hz

// BCD for 0, ..., 9 for the LSD, MSD.
#if VERSION_1_BOARD
uint8_t MSD[10] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
uint8_t LSD[10] = {0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90};
#else
uint8_t LSD[10] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
uint8_t MSD[10] = {0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90};
#endif

/*
 * updateShiftRegister() - This function sets the latchPin to low,
 * then calls the Arduino function 'shiftOut' to shift out contents
 * of variable 'leds' in the shift register before putting the 'latchPin' high again.
 * 
 * I think this code needs to be protected from interrups, but calling 
 * cli() and sei() in here when several bytes are written to a set of 595
 * chips uses about 20uS more than wrapping the set of calls to this in
 * cli() ... sei().
 * 
 * On a scope, it appears that the seialDataPin is left high or low depending 
 * on the last bit value written. I set it LOW so that every call has the
 * same initial condition, although I'm not sure that difference matters to
 * the 595 chips.
 * 
 * I switched to this code over the ShiftRegister74HC595 library because I was
 * getting an odd error where sometimes the values ouput were corrupted. The
 * problem might have been noise on the breadboard or it might have been an
 * issue with interrupts.
 */
void updateShiftRegister(uint8_t data) {
    //cli();
    digitalWrite(latchPin, LOW);
    shiftOut(serialDataPin, clockPin, MSBFIRST, data);
    digitalWrite(latchPin, HIGH);
    digitalWrite(serialDataPin, LOW);
    //sei();
}

void setup() {
    Serial.begin(BAUD_RATE);
    DPRINT("boot\n");
    flush();

    RTC_setup();

    pinMode(latchPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(serialDataPin, OUTPUT);

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

        cli();
        updateShiftRegister(bits[1]);
        updateShiftRegister(bits[0]);
        sei();
    
        delay(digit_time_ms);
        random_time_ms -= digit_time_ms;
    } while (random_time_ms > 0);

    digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
    uint8_t bits[2]; // 1 is the LSD pair, 0 the MSD pair

    hv_ps_adjust();

    if (time_update_handler()) {
        bits[0] = MSD[digit_3] | LSD[digit_2];
        bits[1] = MSD[digit_1] | LSD[digit_0];
        DPRINTV("%d%d:%d%d\n", 0xF0 & bits[0], 0x0F & bits[0], 0xF0 & bits[1], 0x0F & bits[1]);
        cli();
        updateShiftRegister(bits[1]);
        updateShiftRegister(bits[0]);
        sei();
    }
}
