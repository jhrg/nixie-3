
/**
 * @brief RTC interface
 * James Gallagher 8/5/24
 */

#include <Arduino.h>
#include <RTClib.h> // https://github.com/adafruit/RTClib

#include "clock_mode.h"
#include "display_digits.h"
#include "print.h"
#include "pins.h"

#if USE_DS3231
RTC_DS3231 rtc;
#elif USE_DS1307
RTC_DS1307 rtc;
#endif

extern volatile int brightness;  // defined in main.cpp

extern const int brightness_count[];  // See main.cpp for the definition of this. 7/15/25 jhrg

// The global value of time - enables advancing time without I2C use. This
// is global so the value set in setup() will be available initially in the loop().
//
// DateTime cannot be 'volatile' given its definition
DateTime dt;

// The current display digits
volatile int digit_0;
volatile int digit_1;
volatile int digit_2;
volatile int digit_3;
volatile int digit_4;
volatile int digit_5;

void update_display_with_time() {
    struct display_digits digits = digits_from_time(dt.hour(), dt.minute(), dt.second());

    digit_0 = digits.d0;
    digit_1 = digits.d1;
    digit_2 = digits.d2;
    digit_3 = digits.d3;
    digit_4 = digits.d4;
    digit_5 = digits.d5;
}

// mm/dd/yy
void update_display_with_date() {
    struct display_digits digits = digits_from_date(dt.year(), dt.month(), dt.day());

    digit_0 = digits.d0;
    digit_1 = digits.d1;
    digit_2 = digits.d2;
    digit_3 = digits.d3;
    digit_4 = digits.d4;
    digit_5 = digits.d5;
}

/**
 * Print the values of the current digits
 */
void print_digits(bool newline) {
    print("%01d-%01d-%01d-%01d-%01d-%01d\n", digit_5, digit_4, digit_3, digit_2, digit_1, digit_0);
}

/**
 * Print the current time, formatted
 */
void print_time(const DateTime &dt, bool print_newline = false) {
    // or Serial.println(now.toString(buffer));, buffer == YY/MM/DD hh:mm:ss
    print("%02d/%02d/%02d %02d:%02d:%02d", dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second());
    if (print_newline)
        print("\n");
}

// should the clock be checked and the display updated?
volatile bool update_display = false;

volatile bool toggle = false;

// Toggles at the 2Hz ISR rate (i.e. every ~0.5s); used both to decide which
// edge to arm next below and, via blink_off_phase(), to blink the field
// currently selected for editing in Set Time/Set Date mode (NFR-001).
volatile bool blink_phase = true;

/**
 * @brief Record that 1/2 second has elapsed
 *
 * This ISR is triggered on either the rising or falling edge of the 1Hz
 * clock signal (so at a 2Hz rate). This is used to toggle the flashing
 * colon that is the clock digit separator so that it fashes on and off
 * once per second.
 *
 * This ISR is also used to trigger the digit update once per second.
 *
 * The volatile bools toggle and update_display are used to signal other
 * parts of the code that the colons or digits should be updated.
 */
void timer_2HZ_tick_ISR() {
    toggle = true;

    if (blink_phase) {
        attachInterrupt(digitalPinToInterrupt(CLOCK_1HZ), timer_2HZ_tick_ISR, FALLING);
        blink_phase = false;
        update_display = true;
    } else {
        attachInterrupt(digitalPinToInterrupt(CLOCK_1HZ), timer_2HZ_tick_ISR, RISING);
        blink_phase = true;
    }
}

bool blink_off_phase() {
    return !blink_phase;
}

void RTC_setup() {

    pinMode(SEPARATOR, OUTPUT);
    digitalWrite(SEPARATOR, LOW);

    if (rtc.begin()) {
        DPRINT("DS3231/DS1307 RTC Start\n");
    } else {
        DPRINT("Couldn't find RTC\n");
    }

#if ADJUST_TIME
    // Run this here, before serial configuration to shorten the delay
    // between the compiled-in times and the set operation.
    Serial.print("Build date: ");
    Serial.println(__DATE__);
    Serial.print("Build time: ");
    Serial.println(__TIME__);

    DateTime build_time = DateTime(F(__DATE__), F(__TIME__));
    TimeSpan ts(ADJUST_TIME);
    build_time = build_time + ts;
    DateTime now = rtc.now();

    Serial.print(now.unixtime());
    Serial.print(", ");
    Serial.println(build_time.unixtime());

    if (abs(now.unixtime() - build_time.unixtime()) > 60) {
        Serial.print("Adjusting the time: ");
        print_time(build_time, true);

        rtc.adjust(build_time);
    }
#endif

#if USE_DS3231
    rtc.writeSqwPinMode(DS3231_SquareWave1Hz);
#elif USE_DS1307
    rtc.writeSqwPinMode(DS1307_SquareWave1HZ);
#endif

    dt = rtc.now();
    print_time(dt, true);

    cli(); // stop interrupts

    // This is used for the 1Hz pulse from the clock that triggers
    // time updates.
    pinMode(CLOCK_1HZ, INPUT_PULLUP);

    // time_2Hz_tick_ISR() sets a flag that is tested in loop()
    attachInterrupt(digitalPinToInterrupt(CLOCK_1HZ), timer_2HZ_tick_ISR, RISING);

    sei(); // start interrupts
}

void toggle_separator() {
    static bool tick_tok = true;
    if (tick_tok) {
        // turn on separator
        // faster than digitalWrite()
        // PORTB &= ~_BV(SEPARATOR - 8);  // i.e., digitalWrite(SEPARATOR, LOW);
        analogWrite(SEPARATOR, brightness_count[brightness]);
        // digitalWrite(SEPARATOR, HIGH); 7/15//25 jhrg
        tick_tok = false;
    } else {
        // turn off separator
        // PORTB |= _BV(SEPARATOR - 8);  // digitalWrite(SEPARATOR, HIGH);
        digitalWrite(SEPARATOR, LOW);
        tick_tok = true;
    }
}

void commit_hour_or_minute(enum time_field field, enum field_adjust_direction direction) {
    uint8_t hour = dt.hour();
    uint8_t minute = dt.minute();

    if (field == field_hour)
        hour = adjust_hour_value(hour, direction);
    else
        minute = adjust_minute_value(minute, direction);

    dt = DateTime(dt.year(), dt.month(), dt.day(), hour, minute, 0);
    rtc.adjust(dt);
}

void commit_month_day_or_year(enum date_field field, enum field_adjust_direction direction) {
    uint8_t month = dt.month();
    uint8_t day = dt.day();
    uint8_t year_last_two_digits = (uint8_t)(dt.year() - 2000);

    switch (field) {
        case field_month:
            month = adjust_month_value(month, direction);
            break;
        case field_day:
            day = adjust_day_value(day, direction);
            break;
        case field_year:
            year_last_two_digits = adjust_year_value(year_last_two_digits, direction);
            break;
    }

    dt = DateTime(2000 + year_last_two_digits, month, day, dt.hour(), dt.minute(), dt.second());
    rtc.adjust(dt);
}

// Call at least twice a second
bool time_update_handler(enum operating_mode mode) {
    // every 1/2 second
    if (toggle) {
        toggle = false;
        toggle_separator();
    }

    // every second
    if (update_display) {
        update_display = false;
        dt = rtc.now();  // This call takes about 1ms
#if DEBUG
        print_time(dt, true);
#endif
        if (mode == set_date)
            update_display_with_date();
        else
            update_display_with_time();
        return true;
    } else {
        return false;
    }
}
