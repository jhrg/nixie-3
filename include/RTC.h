
/**
 * @brief read from a DS 3231 or 1307 Real Time Clock
 */

// The display digits
extern volatile int digit_0;
extern volatile int digit_1;
extern volatile int digit_2;
extern volatile int digit_3;
extern volatile int digit_4;
extern volatile int digit_5;

void RTC_setup();
bool time_update_handler();
void toggle_separator();
