
#include "pins.h"

void set_date_time_mode_handler();

enum switch_press_duration {
    none,
    quick,      // momentary
    medium_2s,  // 2s
    long_5s     // 5s
};

void input_switch_push();
void input_switch_release();

bool poll_input_button();
void process_input_switch_press();
