
#include <Arduino.h>
#include <PinChangeInterrupt.h>

#include "RTC.h"
#include "mode_switch.h"
#include "print.h"

#define BAUD_RATE 115200

// Choose pins that cannot be used for hardware PWM
int serialDataPin = 8; // DS aka SER (serial data) input
int clockPin = 4;      // SHCP aka SRCLK (shift register clock) input
int latchPin = 7;      // STCP aka RCLK (register clock/latch) input

// OE (Output Enable) and SRCLR (Shift Register Clear)
// OE is pulled low all the time for this code.
// SRCLR is pulled high all the time for this code.

int HV_Control = 5; // PWM brightness 980Hz on pins 5 and 6, otherwise 480Hz

// BCD for 0, ..., 9 for the LSD, MSD.
uint8_t LSD[10] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
uint8_t MSD[10] = {0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90};

/*
 * updateShiftRegister() - This function sets the latchPin to low,
 * then calls the Arduino function 'shiftOut' to shift out contents
 * of variable 'data' in the shift register before putting the 'latchPin' high again.
 *
 * On a scope, it appears that the serialDataPin is left high or low depending
 * on the last bit value written. I set it LOW so that every call has the
 * same initial condition, although I'm not sure that difference matters to
 * the 595 chips.
 *
 * I switched to this code over the ShiftRegister74HC595 library because I was
 * getting an odd error where sometimes the values ouput were corrupted. The
 * problem might have been noise on the breadboard or it might have been an
 * issue with interrupts. NB: It was noise; the HV PS that used the PID controller
 * was noisy and that was fixed by using a better HV PS. That also meant tha
 * the PID controller could be dumped.
 */
void updateShiftRegister(uint8_t data) {
    digitalWrite(latchPin, LOW);
    shiftOut(serialDataPin, clockPin, MSBFIRST, data);
    digitalWrite(latchPin, HIGH);
    digitalWrite(serialDataPin, LOW);
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

    // Flash random digits at start up.
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

    // hv_ps_adjust();

    if (time_update_handler()) {
        bits[0] = MSD[digit_3] | LSD[digit_2];
        bits[1] = MSD[digit_1] | LSD[digit_0];
        // I don't know for sure that these calls are needed. They seem to
        // do no harm.
        cli();
        updateShiftRegister(bits[1]);
        updateShiftRegister(bits[0]);
        sei();
    }
}
