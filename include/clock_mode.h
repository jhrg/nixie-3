
/**
 * @brief Operating-mode state machine and per-field value adjustment for
 * Set Time / Set Date mode.
 */

#ifndef clock_mode_h_
#define clock_mode_h_

#include <stdint.h>

enum operating_mode { running, set_time, set_date };

enum time_field { field_hour, field_minute };

enum date_field { field_month, field_day, field_year };

enum field_adjust_direction { field_increment, field_decrement };

/**
 * @brief Advance the operating mode: running -> set_time -> set_date -> running.
 */
enum operating_mode next_operating_mode(enum operating_mode mode);

/**
 * @brief Toggle the selected time field: hour <-> minute.
 */
enum time_field next_time_field(enum time_field field);

/**
 * @brief Advance the selected date field: month -> day -> year -> month.
 */
enum date_field next_date_field(enum date_field field);

/**
 * @brief Adjust an hour value by one, wrapping 0-23 in either direction.
 */
uint8_t adjust_hour_value(uint8_t hour, enum field_adjust_direction direction);

/**
 * @brief Adjust a minute value by one, wrapping 0-59 in either direction.
 */
uint8_t adjust_minute_value(uint8_t minute, enum field_adjust_direction direction);

/**
 * @brief Adjust a month value by one, wrapping 1-12 in either direction.
 */
uint8_t adjust_month_value(uint8_t month, enum field_adjust_direction direction);

/**
 * @brief Adjust a day-of-month value by one, wrapping 1-31 in either direction
 * (no calendar/leap-year awareness).
 */
uint8_t adjust_day_value(uint8_t day, enum field_adjust_direction direction);

/**
 * @brief Adjust a two-digit year offset by one, wrapping 0-99 in either direction.
 */
uint8_t adjust_year_value(uint8_t year_last_two_digits, enum field_adjust_direction direction);

/**
 * @brief Fixed century shown as "20" in the four-digit year 20YY (years 2000-2099).
 */
constexpr uint8_t YEAR_CENTURY_TENS_DIGIT = 2;
constexpr uint8_t YEAR_CENTURY_ONES_DIGIT = 0;

/**
 * @brief Hardware blanking value for a Nixie digit pair (both nibbles 0xF).
 */
constexpr uint8_t DIGIT_PAIR_BLANK = 0xFF;

/**
 * @brief Blank a digit pair's bits when it is the selected field and the blink
 * phase is off; otherwise return the bits unchanged.
 */
uint8_t blank_if_selected(uint8_t digit_pair_bits, bool is_selected, bool blink_off);

/**
 * @brief Decide whether the display digits must be re-read from the RTC now: on the
 * 1 Hz refetch tick, or whenever the requested digit kind (date vs. time) differs
 * from the kind last computed (BUG-004).
 */
bool needs_refetch(bool refetch_tick, bool want_date, bool have_date);

#endif
