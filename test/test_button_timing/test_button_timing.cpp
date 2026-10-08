#include <unity.h>

#include "button_timing.h"

void setUp(void) {}
void tearDown(void) {}

void test_quick_below_2s(void) {
    TEST_ASSERT_EQUAL(quick, classify_press_duration(1999));
}

void test_quick_at_2s_boundary(void) {
    TEST_ASSERT_EQUAL(quick, classify_press_duration(SWITCH_PRESS_2S));
}

void test_medium_just_above_2s(void) {
    TEST_ASSERT_EQUAL(medium_2s, classify_press_duration(SWITCH_PRESS_2S + 1));
}

void test_medium_at_5s_boundary(void) {
    TEST_ASSERT_EQUAL(medium_2s, classify_press_duration(SWITCH_PRESS_5S));
}

void test_long_just_above_5s(void) {
    TEST_ASSERT_EQUAL(long_5s, classify_press_duration(SWITCH_PRESS_5S + 1));
}

void test_quick_on_zero_or_negative(void) {
    // A millis() rollover can make elapsed <= 0; this must still resolve to
    // quick, matching the pre-extraction inline logic it replaced.
    TEST_ASSERT_EQUAL(quick, classify_press_duration(0));
    TEST_ASSERT_EQUAL(quick, classify_press_duration(-100));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_quick_below_2s);
    RUN_TEST(test_quick_at_2s_boundary);
    RUN_TEST(test_medium_just_above_2s);
    RUN_TEST(test_medium_at_5s_boundary);
    RUN_TEST(test_long_just_above_5s);
    RUN_TEST(test_quick_on_zero_or_negative);
    return UNITY_END();
}
