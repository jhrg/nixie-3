#include "mode_switch2.h"

enum display_mode next_display_mode(enum display_mode display_mode) {
    switch (display_mode) {
        case hh_mm:
            return mm_ss;
        case mm_ss:
            return date_display;
        case date_display:
            return hh_mm;
    }
    return hh_mm;
}

uint8_t next_date_cycle_tick(uint8_t tick) {
    return (tick + 1) % DATE_CYCLE_TOTAL_TICKS;
}

bool date_cycle_shows_year(uint8_t tick) {
    return tick >= DATE_CYCLE_MONTH_DAY_TICKS;
}
