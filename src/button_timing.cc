
#include "button_timing.h"

enum switch_press_duration classify_press_duration(long elapsed_ms) {
    if (elapsed_ms > SWITCH_PRESS_5S)
        return long_5s;
    else if (elapsed_ms > SWITCH_PRESS_2S)
        return medium_2s;
    else
        return quick;
}
