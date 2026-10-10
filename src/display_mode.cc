
#include "mode_switch2.h"

enum display_mode toggle_display_mode(enum display_mode display_mode) {
    switch (display_mode) {
        case mm_ss:
            display_mode = hh_mm;
            break;

        case hh_mm:
            display_mode = mm_ss;
            break;

    };

    return display_mode;
}
