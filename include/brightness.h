
/**
 * @brief Brightness-level index arithmetic, independent of the PWM hardware.
 */

#ifndef brightness_h_
#define brightness_h_

#include <stdint.h>

/**
 * @brief Compute the next brightness level index, wrapping around.
 *
 * Returns 0 if current_index is the last valid index (level_count - 1),
 * otherwise current_index + 1.
 */
uint8_t next_brightness_index(uint8_t current_index, uint8_t level_count);

#endif
