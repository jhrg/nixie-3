#include <unity.h>

#include "clock_mode.h"

void setUp(void) {}
void tearDown(void) {}

void test_operating_mode_cycles_through_all_three(void) {
    enum operating_mode mode = running;
    mode = next_operating_mode(mode);
    TEST_ASSERT_EQUAL(set_time, mode);
    mode = next_operating_mode(mode);
    TEST_ASSERT_EQUAL(set_date, mode);
    mode = next_operating_mode(mode);
    TEST_ASSERT_EQUAL(running, mode);
}

void test_time_field_toggles_hour_and_minute(void) {
    TEST_ASSERT_EQUAL(field_minute, next_time_field(field_hour));
    TEST_ASSERT_EQUAL(field_hour, next_time_field(field_minute));
}

void test_date_field_cycles_through_all_three(void) {
    enum date_field field = field_month;
    field = next_date_field(field);
    TEST_ASSERT_EQUAL(field_day, field);
    field = next_date_field(field);
    TEST_ASSERT_EQUAL(field_year, field);
    field = next_date_field(field);
    TEST_ASSERT_EQUAL(field_month, field);
}

void test_hour_wraps_incrementing(void) {
    TEST_ASSERT_EQUAL(0, adjust_hour_value(23, field_increment));
    TEST_ASSERT_EQUAL(13, adjust_hour_value(12, field_increment));
}

void test_hour_wraps_decrementing(void) {
    TEST_ASSERT_EQUAL(23, adjust_hour_value(0, field_decrement));
    TEST_ASSERT_EQUAL(11, adjust_hour_value(12, field_decrement));
}

void test_minute_wraps_incrementing(void) {
    TEST_ASSERT_EQUAL(0, adjust_minute_value(59, field_increment));
}

void test_minute_wraps_decrementing(void) {
    TEST_ASSERT_EQUAL(59, adjust_minute_value(0, field_decrement));
}

void test_month_wraps_incrementing(void) {
    TEST_ASSERT_EQUAL(1, adjust_month_value(12, field_increment));
}

void test_month_wraps_decrementing(void) {
    TEST_ASSERT_EQUAL(12, adjust_month_value(1, field_decrement));
}

void test_day_wraps_incrementing(void) {
    TEST_ASSERT_EQUAL(1, adjust_day_value(31, field_increment));
}

void test_day_wraps_decrementing(void) {
    TEST_ASSERT_EQUAL(31, adjust_day_value(1, field_decrement));
}

void test_year_wraps_incrementing(void) {
    TEST_ASSERT_EQUAL(0, adjust_year_value(99, field_increment));
}

void test_year_wraps_decrementing(void) {
    TEST_ASSERT_EQUAL(99, adjust_year_value(0, field_decrement));
}

void test_blank_if_selected_blinks_off_when_selected(void) {
    TEST_ASSERT_EQUAL(DIGIT_PAIR_BLANK, blank_if_selected(0x12, true, true));
}

void test_blank_if_selected_lit_when_selected_and_blink_on(void) {
    TEST_ASSERT_EQUAL(0x12, blank_if_selected(0x12, true, false));
}

void test_blank_if_selected_lit_when_not_selected(void) {
    TEST_ASSERT_EQUAL(0x12, blank_if_selected(0x12, false, true));
    TEST_ASSERT_EQUAL(0x12, blank_if_selected(0x12, false, false));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_operating_mode_cycles_through_all_three);
    RUN_TEST(test_time_field_toggles_hour_and_minute);
    RUN_TEST(test_date_field_cycles_through_all_three);
    RUN_TEST(test_hour_wraps_incrementing);
    RUN_TEST(test_hour_wraps_decrementing);
    RUN_TEST(test_minute_wraps_incrementing);
    RUN_TEST(test_minute_wraps_decrementing);
    RUN_TEST(test_month_wraps_incrementing);
    RUN_TEST(test_month_wraps_decrementing);
    RUN_TEST(test_day_wraps_incrementing);
    RUN_TEST(test_day_wraps_decrementing);
    RUN_TEST(test_year_wraps_incrementing);
    RUN_TEST(test_year_wraps_decrementing);
    RUN_TEST(test_blank_if_selected_blinks_off_when_selected);
    RUN_TEST(test_blank_if_selected_lit_when_selected_and_blink_on);
    RUN_TEST(test_blank_if_selected_lit_when_not_selected);
    return UNITY_END();
}
