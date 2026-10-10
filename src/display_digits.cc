
#include "display_digits.h"

struct display_digits digits_from_time(int hour, int minute, int second) {
    struct display_digits digits;

    digits.d0 = second % 10;
    digits.d1 = second / 10;

    digits.d2 = minute % 10;
    digits.d3 = minute / 10;

    digits.d4 = hour % 10;
    digits.d5 = hour / 10;

    return digits;
}

struct display_digits digits_from_date(int year, int month, int day) {
    struct display_digits digits;

    digits.d0 = year % 10;
    digits.d1 = (year - 2000) / 10;

    digits.d2 = day % 10;
    digits.d3 = day / 10;

    digits.d4 = month % 10;
    digits.d5 = month / 10;

    return digits;
}
