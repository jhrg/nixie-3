
/**
 * @brief Classify a button hold duration into a press-duration category.
 */

#ifndef button_timing_h_
#define button_timing_h_

#include "mode_switch2.h"

#define SWITCH_PRESS_2S 2000    // 2 Seconds
#define SWITCH_PRESS_5S 5000    // 5 S
#define SWITCH_PRESS_10S 10000  // 10 S

/**
 * @brief Classify an elapsed button-hold duration.
 *
 * quick: elapsed_ms <= SWITCH_PRESS_2S (also covers zero/negative values, e.g.
 * from a millis() rollover, matching the pre-extraction behavior). medium_2s:
 * SWITCH_PRESS_2S < elapsed_ms <= SWITCH_PRESS_5S. long_5s: elapsed_ms >
 * SWITCH_PRESS_5S.
 */
enum switch_press_duration classify_press_duration(long elapsed_ms);

#endif
