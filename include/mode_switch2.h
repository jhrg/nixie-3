
#ifndef mode_switch_h_
#define mode_switch_h_

#include <stdint.h>

enum switch_press_duration {
    none,
    quick,      // momentary
    medium_2s,  // 2s
    long_5s     // 5s
};

enum display_mode {
    mm_ss,
    hh_mm,
    date_display
};

// Date display mode cycle, counted in 2 Hz half-second ticks: 2 s of MM:DD, then 1 s of 20YY.
constexpr uint8_t DATE_CYCLE_MONTH_DAY_TICKS = 4;
constexpr uint8_t DATE_CYCLE_TOTAL_TICKS = 6;

enum switch_press_duration read_button_1();
enum switch_press_duration read_button_2();
void mode_switch_setup();

/**
 * @brief Advance the display mode: hh_mm -> mm_ss -> date_display -> hh_mm.
 */
enum display_mode next_display_mode(enum display_mode display_mode);

/**
 * @brief Advance the date-cycle half-second tick, wrapping at DATE_CYCLE_TOTAL_TICKS.
 */
uint8_t next_date_cycle_tick(uint8_t tick);

/**
 * @brief True when the date-cycle tick falls in the 20YY (year) part of the cycle.
 */
bool date_cycle_shows_year(uint8_t tick);

#endif
