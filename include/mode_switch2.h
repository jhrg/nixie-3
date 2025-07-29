
#ifndef mode_switch_h_
#define mode_switch_h_

enum switch_press_duration {
    none,
    quick,      // momentary
    medium_2s,  // 2s
    long_5s     // 5s
};

enum display_mode {
    mm_ss,
    hh_mm
};

enum switch_press_duration read_button_1();
enum switch_press_duration read_button_2();
void mode_switch_setup();

#endif
