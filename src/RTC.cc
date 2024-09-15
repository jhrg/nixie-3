
/**
 * @brief RTC interface
 * James Gallagher 8/5/24
 */

#include <Arduino.h>
#include <RTClib.h> // https://github.com/adafruit/RTClib

#include "print.h"

#define CLOCK_1HZ 2

// SEPARATOR can be any pin between 8 and 13 inclusive (PORT B)
#define SEPARATOR 9

#if USE_DS3231
RTC_DS3231 rtc;
#elif USE_DS1307
RTC_DS1307 rtc;
#endif

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
    digit_0 = dt.second() % 10;
    digit_1 = dt.second() / 10;

    digit_2 = dt.minute() % 10;
    digit_3 = dt.minute() / 10;

    digit_4 = dt.hour() % 10;
    digit_5 = dt.hour() / 10;
}

// mm/dd/yy
void update_display_with_date() {
    digit_0 = dt.year() % 10;
    digit_1 = (dt.year() - 2000) / 10;

    digit_2 = dt.day() % 10;
    digit_3 = dt.day() / 10;

    digit_4 = dt.month() % 10;
    digit_5 = dt.month() / 10;
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

    static volatile bool tick_tok = true;

    if (tick_tok) {
        tick_tok = false;
        update_display = true;
    } else {
        tick_tok = true;
    }
}

void RTC_setup() {

    pinMode(SEPARATOR, OUTPUT);
    digitalWrite(SEPARATOR, LOW);

    if (rtc.begin()) {
        DPRINT("DS3131/DS1307 RTC Start\n");
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
    attachInterrupt(digitalPinToInterrupt(CLOCK_1HZ), timer_2HZ_tick_ISR, CHANGE);

    sei(); // start interrupts
}

void toggle_separator() {
    static bool tick_tok = true;
    if (tick_tok) {
        // turn on separator
        // faster than digitalWrite()
        PORTB &= ~_BV(SEPARATOR - 8);  // i.e., digitalWrite(SEPARATOR, LOW);
        tick_tok = false;
    } else {
        // turn off separator
        PORTB |= _BV(SEPARATOR - 8);  // digitalWrite(SEPARATOR, HIGH);
        tick_tok = true;
    }
}

// Call at least twice a second
bool time_update_handler() {
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
        update_display_with_time();
        return true;
    } else {
        return false;
    }
}
