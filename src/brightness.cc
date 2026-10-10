
#include "brightness.h"

uint8_t next_brightness_index(uint8_t current_index, uint8_t level_count) {
    return (current_index == level_count - 1) ? 0 : current_index + 1;
}
