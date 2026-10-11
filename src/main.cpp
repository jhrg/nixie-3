
#include <Arduino.h>
#include <PinChangeInterrupt.h>

#include "RTC.h"
#include "brightness.h"
#include "clock_mode.h"
#include "mode_switch2.h"
#include "pins.h"
#include "print.h"

#define BAUD_RATE 115200

volatile int brightness = 0;
// Because this is C++, the definition must use 'extern' for this to be visible
// in another source file (i.e., translation unit). See RTC.cc. 7/15/25 jhrg
extern const int brightness_count[] = {255, 128, 76, 24, 0};

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
 * was noisy and that was fixed by using a better HV PS. That also meant that
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
    brightness = next_brightness_index(brightness, sizeof(brightness_count) / sizeof(brightness_count[0]));
    DPRINTV("brightness: %d\n", brightness);
    analogWrite(HV_PWM_CONTROL, brightness_count[brightness]);
}

void loop() {
    uint8_t bits[2]; // 1 is the LSD pair, 0 the MSD pair
    static enum display_mode the_display_mode = mm_ss;  // initialized to mm_ss
    static enum operating_mode the_operating_mode = running;
    static enum time_field the_time_field = field_hour;
    static enum date_field the_date_field = field_month;

    switch (read_button_1()) {
        case quick:
            if (the_operating_mode == running) {
                input_switch_quick_press();
            } else if (the_operating_mode == set_time) {
                commit_hour_or_minute(the_time_field, field_increment);
            } else {
                commit_month_day_or_year(the_date_field, field_increment);
            }
            break;
        case medium_2s:
            if (the_operating_mode == set_time)
                the_time_field = next_time_field(the_time_field);
            else if (the_operating_mode == set_date)
                the_date_field = next_date_field(the_date_field);
            break;  // no-op in Running mode; reserved for future task/feature there
        case long_5s:
            break;  // no-op; reserved for future task/feature
        default:
            break;
    }

    switch (read_button_2()) {
        case quick:
            if (the_operating_mode == running) {
                the_display_mode = toggle_display_mode(the_display_mode);
                DPRINTV("display mode: %s\n", the_display_mode == mm_ss ? "MM:SS" : "HH:MM");
            } else if (the_operating_mode == set_time) {
                commit_hour_or_minute(the_time_field, field_decrement);
            } else {
                commit_month_day_or_year(the_date_field, field_decrement);
            }
            break;
        case medium_2s:
            the_operating_mode = next_operating_mode(the_operating_mode);
            if (the_operating_mode == set_time)
                the_time_field = field_hour;
            else if (the_operating_mode == set_date)
                the_date_field = field_month;
            break;
        case long_5s:
            break;  // no-op; reserved for future task/feature
        default:
            break;
    }

    if (time_update_handler(the_operating_mode)) {
        switch (the_operating_mode) {
            case running:
                switch (the_display_mode) {
                    case mm_ss:
                        bits[0] = MSD[digit_3] | LSD[digit_2];
                        bits[1] = MSD[digit_1] | LSD[digit_0];
                        break;

                    case hh_mm:
                        bits[0] = MSD[digit_5] | LSD[digit_4];
                        bits[1] = MSD[digit_3] | LSD[digit_2];
                        break;
                }
                break;

            case set_time:
                bits[0] = blank_if_selected(MSD[digit_5] | LSD[digit_4], the_time_field == field_hour, blink_off_phase());
                bits[1] = blank_if_selected(MSD[digit_3] | LSD[digit_2], the_time_field == field_minute, blink_off_phase());
                break;

            case set_date: {
                uint8_t month_or_year_bits = (the_date_field == field_year) ? (MSD[digit_1] | LSD[digit_0])
                                                                             : (MSD[digit_5] | LSD[digit_4]);
                bits[0] = blank_if_selected(month_or_year_bits,
                                             the_date_field == field_month || the_date_field == field_year,
                                             blink_off_phase());
                bits[1] = blank_if_selected(MSD[digit_3] | LSD[digit_2], the_date_field == field_day, blink_off_phase());
                break;
            }
        }

        // I don't know for sure that these cli/sei calls are needed. They seem to do no harm.
        cli();
        updateShiftRegister(bits[1]);
        updateShiftRegister(bits[0]);
        sei();
    }
}
