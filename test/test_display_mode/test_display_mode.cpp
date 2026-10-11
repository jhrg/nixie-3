#include <unity.h>

#include "mode_switch2.h"

void setUp(void) {}
void tearDown(void) {}

void test_hh_mm_advances_to_mm_ss(void) {
    TEST_ASSERT_EQUAL(mm_ss, next_display_mode(hh_mm));
}

void test_mm_ss_advances_to_date(void) {
    TEST_ASSERT_EQUAL(date_display, next_display_mode(mm_ss));
}

void test_date_advances_to_hh_mm(void) {
    TEST_ASSERT_EQUAL(hh_mm, next_display_mode(date_display));
}

void test_three_advances_return_to_start(void) {
    enum display_mode start = hh_mm;
    TEST_ASSERT_EQUAL(start, next_display_mode(next_display_mode(next_display_mode(start))));
}

void test_date_cycle_shows_year_only_in_last_two_ticks(void) {
    for (uint8_t t = 0; t < 4; t++)
        TEST_ASSERT_FALSE(date_cycle_shows_year(t));
    TEST_ASSERT_TRUE(date_cycle_shows_year(4));
    TEST_ASSERT_TRUE(date_cycle_shows_year(5));
}

void test_date_cycle_tick_wraps(void) {
    TEST_ASSERT_EQUAL(1, next_date_cycle_tick(0));
    TEST_ASSERT_EQUAL(5, next_date_cycle_tick(4));
    TEST_ASSERT_EQUAL(0, next_date_cycle_tick(5));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_hh_mm_advances_to_mm_ss);
    RUN_TEST(test_mm_ss_advances_to_date);
    RUN_TEST(test_date_advances_to_hh_mm);
    RUN_TEST(test_three_advances_return_to_start);
    RUN_TEST(test_date_cycle_shows_year_only_in_last_two_ticks);
    RUN_TEST(test_date_cycle_tick_wraps);
    return UNITY_END();
}
