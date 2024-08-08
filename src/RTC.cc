
/**
 * @brief RTC interface
 * James Gallagher 8/5/24
 */

#include <Arduino.h>
#include <PinChangeInterrupt.h>
#include <RTClib.h> // https://github.com/adafruit/RTClib

#include "print.h"

#define CLOCK_QUERY_INTERVAL 12 // seconds

#define CLOCK_1HZ 2

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
#if DEBUG
    print("%01d-%01d-%01d-%01d-%01d-%01d\n", digit_5, digit_4, digit_3, digit_2, digit_1, digit_0);
#endif
}

/**
 * Print the current time, formatted
 */
void print_time(DateTime dt, bool print_newline = false) {
#if DEBUG
    // or Serial.println(now.toString(buffer));, buffer == YY/MM/DD hh:mm:ss
    print("%02d/%02d/%02d %02d:%02d:%02d", dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second());
    if (print_newline)
        print("\n");
#endif
}

// Set HIGH when the CLOCK_QUERY_INTERVAL seconds have elapsed and
// the RTC should be accessed.
volatile bool get_time = false;

// Set HIGH when the 1 second interrupt been triggered by the clock,
// Used in main_mode_handler()
volatile bool update_display = false;

/**
 * @brief Record that one second has elapsed
 */
void timer_1HZ_tick_ISR() {
    static volatile int tick_count = 0;

    tick_count++;

    if (tick_count >= CLOCK_QUERY_INTERVAL) {
        // update time using I2C access to the clock
        tick_count = 0;
        get_time = true;
    }

    // when get_time is false, main_mode_handler() adds one second to the global
    // time (dt) when update_display is true.
    update_display = true;
}

void RTC_setup() {
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

    // time_1Hz_tick() sets a flag that is tested in loop()
    attachInterrupt(digitalPinToInterrupt(CLOCK_1HZ), timer_1HZ_tick_ISR, RISING);

    sei(); // start interrupts
}

// Call at least 1/s
bool time_update_handler() {
    static TimeSpan ts(1); // a one-second time span

    if (get_time) {
        DPRINT("Get time\n");
        get_time = false;
        dt = rtc.now(); // This call takes about 1ms
        print_time(dt, true);
        update_display_with_time();
        return true;
    } else if (update_display) {
        update_display = false;
        dt = dt + ts; // Advance 'dt' by one second
        print_time(dt, true);
        update_display_with_time();
        return true;
    }
    else {
        return false;
    }
}
