#include <unity.h>

#include "display_digits.h"

void setUp(void) {}
void tearDown(void) {}

void test_digits_from_time_mid_range(void) {
    struct display_digits digits = digits_from_time(14, 37, 59);
    TEST_ASSERT_EQUAL(9, digits.d0);  // seconds ones
    TEST_ASSERT_EQUAL(5, digits.d1);  // seconds tens
    TEST_ASSERT_EQUAL(7, digits.d2);  // minutes ones
    TEST_ASSERT_EQUAL(3, digits.d3);  // minutes tens
    TEST_ASSERT_EQUAL(4, digits.d4);  // hours ones
    TEST_ASSERT_EQUAL(1, digits.d5);  // hours tens
}

void test_digits_from_time_zero(void) {
    struct display_digits digits = digits_from_time(0, 0, 0);
    TEST_ASSERT_EQUAL(0, digits.d0);
    TEST_ASSERT_EQUAL(0, digits.d1);
    TEST_ASSERT_EQUAL(0, digits.d2);
    TEST_ASSERT_EQUAL(0, digits.d3);
    TEST_ASSERT_EQUAL(0, digits.d4);
    TEST_ASSERT_EQUAL(0, digits.d5);
}

void test_digits_from_time_hour_23(void) {
    struct display_digits digits = digits_from_time(23, 0, 0);
    TEST_ASSERT_EQUAL(3, digits.d4);
    TEST_ASSERT_EQUAL(2, digits.d5);
}

void test_digits_from_date_two_digit_year_2000(void) {
    struct display_digits digits = digits_from_date(2000, 3, 7);
    TEST_ASSERT_EQUAL(0, digits.d0);  // year ones
    TEST_ASSERT_EQUAL(0, digits.d1);  // year tens
    TEST_ASSERT_EQUAL(7, digits.d2);  // day ones
    TEST_ASSERT_EQUAL(0, digits.d3);  // day tens
    TEST_ASSERT_EQUAL(3, digits.d4);  // month ones
    TEST_ASSERT_EQUAL(0, digits.d5);  // month tens
}

void test_digits_from_date_two_digit_year_2099(void) {
    struct display_digits digits = digits_from_date(2099, 12, 25);
    TEST_ASSERT_EQUAL(9, digits.d0);  // year ones
    TEST_ASSERT_EQUAL(9, digits.d1);  // year tens
    TEST_ASSERT_EQUAL(5, digits.d2);  // day ones
    TEST_ASSERT_EQUAL(2, digits.d3);  // day tens
    TEST_ASSERT_EQUAL(2, digits.d4);  // month ones
    TEST_ASSERT_EQUAL(1, digits.d5);  // month tens
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_digits_from_time_mid_range);
    RUN_TEST(test_digits_from_time_zero);
    RUN_TEST(test_digits_from_time_hour_23);
    RUN_TEST(test_digits_from_date_two_digit_year_2000);
    RUN_TEST(test_digits_from_date_two_digit_year_2099);
    return UNITY_END();
}
