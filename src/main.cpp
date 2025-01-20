
#include <Arduino.h>
#include <PinChangeInterrupt.h>

#include "RTC.h"
#include "mode_switch2.h"
#include "pins.h"
#include "print.h"

#define BAUD_RATE 115200

volatile int brightness = 0;

const int brightness_count[] = {255, 128, 76, 24, 0};

// BCD for 0, ..., 9 for the LSD, MSD.
uint8_t LSD[10] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
uint8_t MSD[10] = {0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90};

/*
 * updateShiftRegister() - This function sets the REGISTER_CLK pin to low,
 * then calls the Arduino function 'shiftOut' to shift out contents
 * of variable 'data' in the shift register before putting 'REGISTER_CLK' high again.
 *
 * On a scope, it appears that the SERIAL_DATA pin is left high or low depending
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
    digitalWrite(REGISTER_CLK, LOW);
    shiftOut(SERIAL_DATA, SERIAL_CLK, MSBFIRST, data);
    digitalWrite(REGISTER_CLK, HIGH);
    digitalWrite(SERIAL_DATA, LOW);
}

void setup() {
    Serial.begin(BAUD_RATE);
    DPRINT("boot\n");
    flush();

    RTC_setup();
    mode_switch_setup();

    cli();
    pinMode(REGISTER_CLK, OUTPUT);
    pinMode(SERIAL_CLK, OUTPUT);
    pinMode(SERIAL_DATA, OUTPUT);

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(HV_PWM_CONTROL, OUTPUT);
#if 0
    pinMode(INPUT_SWITCH, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(INPUT_SWITCH), input_switch_push, RISING);
#endif
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(HV_PWM_CONTROL, HIGH);  // Start out bright
    sei();

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

void input_switch_quick_press() {
    brightness = (brightness == sizeof(brightness_count) / sizeof(brightness_count[0]) - 1) ? 0 : brightness + 1;
    DPRINTV("brightness: %d\n", brightness);
    analogWrite(HV_PWM_CONTROL, brightness_count[brightness]);
}

void input_switch_medium_press(enum display_mode &the_display_mode) {
    switch (the_display_mode) {
        case mm_ss:
            the_display_mode = hh_mm;
            break;

        case hh_mm:
            the_display_mode = mm_ss;
            break;
    };
    DPRINTV("display mode: %s\n", the_display_mode == mm_ss ? "MM:SS" : "HH:MM");
}

void loop() {
    uint8_t bits[2]; // 1 is the LSD pair, 0 the MSD pair
    enum display_mode the_display_mode;  // initialized to mm_ss

    // hv_ps_adjust();

    switch (read_button()) {
        case quick:
            input_switch_quick_press();
            break;
        case medium_2s:
            input_switch_medium_press(the_display_mode);
            break;
        default:
            break;
    }

    if (time_update_handler()) {
        switch (the_display_mode) {
            case mm_ss:
                bits[0] = MSD[digit_3] | LSD[digit_2];
                bits[1] = MSD[digit_1] | LSD[digit_0];
                break;

            case hh_mm:
                bits[0] = MSD[digit_5] | LSD[digit_4];
                bits[1] = MSD[digit_3] | LSD[digit_2];
                break;
        };

         // I don't know for sure that these calls are needed. They seem to
        // do no harm.
        cli();
        updateShiftRegister(bits[1]);
        updateShiftRegister(bits[0]);
        sei();
    }
}
