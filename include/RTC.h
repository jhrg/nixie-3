
/**
 * @brief read from a DS 3231 or 1307 Real Time Clock
 */

#ifndef rtc_h_
#define rtc_h_

#include "clock_mode.h"

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
 * second. mode selects whether the time or the date is rendered into the
 * digits (set_date renders the date; running/set_time render the time).
 */
bool time_update_handler(enum operating_mode mode);

void toggle_separator();

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
