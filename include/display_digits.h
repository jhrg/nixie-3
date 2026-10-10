
/**
 * @brief Decompose a time or date into the six Nixie display digits.
 */

#ifndef display_digits_h_
#define display_digits_h_

struct display_digits {
    int d0;
    int d1;
    int d2;
    int d3;
    int d4;
    int d5;
};

/**
 * @brief Split hour/minute/second into display digits (seconds/minutes/hours,
 * ones then tens).
 */
struct display_digits digits_from_time(int hour, int minute, int second);

/**
 * @brief Split year/month/day into display digits (mm/dd/yy, ones then tens).
 *
 * year is the full year (e.g. 2026); only its last two digits are displayed.
 */
struct display_digits digits_from_date(int year, int month, int day);

#endif
