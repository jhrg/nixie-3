#include <unity.h>

#include "brightness.h"

void setUp(void) {}
void tearDown(void) {}

void test_wraps_from_last_index_to_zero(void) {
    TEST_ASSERT_EQUAL(0, next_brightness_index(4, 5));
}

void test_increments_mid_range(void) {
    TEST_ASSERT_EQUAL(1, next_brightness_index(0, 5));
    TEST_ASSERT_EQUAL(3, next_brightness_index(2, 5));
}

void test_single_level_always_wraps_to_zero(void) {
    TEST_ASSERT_EQUAL(0, next_brightness_index(0, 1));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_wraps_from_last_index_to_zero);
    RUN_TEST(test_increments_mid_range);
    RUN_TEST(test_single_level_always_wraps_to_zero);
    return UNITY_END();
}
