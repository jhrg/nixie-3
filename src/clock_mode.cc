
#include "clock_mode.h"

enum operating_mode next_operating_mode(enum operating_mode mode) {
    switch (mode) {
        case running:
            return set_time;
        case set_time:
            return set_date;
        case set_date:
            return running;
    }
    return running;
}

enum time_field next_time_field(enum time_field field) {
    return (field == field_hour) ? field_minute : field_hour;
}

enum date_field next_date_field(enum date_field field) {
    switch (field) {
        case field_month:
            return field_day;
        case field_day:
            return field_year;
        case field_year:
            return field_month;
    }
    return field_month;
}

static uint8_t adjust_wrapping(uint8_t value, uint8_t min_value, uint8_t max_value,
                                enum field_adjust_direction direction) {
    if (direction == field_increment)
        return (value == max_value) ? min_value : value + 1;
    else
        return (value == min_value) ? max_value : value - 1;
}

uint8_t adjust_hour_value(uint8_t hour, enum field_adjust_direction direction) {
    return adjust_wrapping(hour, 0, 23, direction);
}

uint8_t adjust_minute_value(uint8_t minute, enum field_adjust_direction direction) {
    return adjust_wrapping(minute, 0, 59, direction);
}

uint8_t adjust_month_value(uint8_t month, enum field_adjust_direction direction) {
    return adjust_wrapping(month, 1, 12, direction);
}

uint8_t adjust_day_value(uint8_t day, enum field_adjust_direction direction) {
    return adjust_wrapping(day, 1, 31, direction);
}

uint8_t adjust_year_value(uint8_t year_last_two_digits, enum field_adjust_direction direction) {
    return adjust_wrapping(year_last_two_digits, 0, 99, direction);
}

uint8_t blank_if_selected(uint8_t digit_pair_bits, bool is_selected, bool blink_off) {
    return (is_selected && blink_off) ? DIGIT_PAIR_BLANK : digit_pair_bits;
}
