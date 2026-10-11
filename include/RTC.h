
/**
 * @brief read from a DS 3231 or 1307 Real Time Clock
 */

#ifndef rtc_h_
#define rtc_h_

#include "clock_mode.h"
#include "mode_switch2.h"

// The display digits
extern volatile int digit_0;
extern volatile int digit_1;
extern volatile int digit_2;
extern volatile int digit_3;
extern volatile int digit_4;
extern volatile int digit_5;

void RTC_setup();

/**
 * @brief Poll the RTC and refresh the display digits; call at least twice a
 * second. The date is rendered into the digits in set_date mode, and in
 * running mode when display is date_display; otherwise the time is. The digits
 * are re-read at once when that kind changes (BUG-004).
 */
bool time_update_handler(enum operating_mode mode, enum display_mode display);

void toggle_separator();

/**
 * @brief Drive the colon separator solid (at the current brightness) or off.
 * Call after RTC_setup().
 */
void set_separator(bool on);

/**
 * @brief Apply one increment/decrement to the hour or minute field and write
 * the result to the RTC immediately, resetting seconds to :00.
 */
void commit_hour_or_minute(enum time_field field, enum field_adjust_direction direction);

/**
 * @brief Apply one increment/decrement to the month, day, or year field and
 * write the result to the RTC immediately.
 */
void commit_month_day_or_year(enum date_field field, enum field_adjust_direction direction);

/**
 * @brief True during the "off" half of the ~1 Hz blink cycle used to blank the
 * field currently selected for editing (NFR-001).
 */
bool blink_off_phase();

#endif
