
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

enum switch_press_duration read_button();
void mode_switch_setup();
