#include <unity.h>

#include "mode_switch2.h"

void setUp(void) {}
void tearDown(void) {}

void test_toggle_from_mm_ss_to_hh_mm(void) {
    TEST_ASSERT_EQUAL(hh_mm, toggle_display_mode(mm_ss));
}

void test_toggle_from_hh_mm_to_mm_ss(void) {
    TEST_ASSERT_EQUAL(mm_ss, toggle_display_mode(hh_mm));
}

void test_toggle_is_its_own_inverse(void) {
    enum display_mode start = mm_ss;
    TEST_ASSERT_EQUAL(start, toggle_display_mode(toggle_display_mode(start)));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_toggle_from_mm_ss_to_hh_mm);
    RUN_TEST(test_toggle_from_hh_mm_to_mm_ss);
    RUN_TEST(test_toggle_is_its_own_inverse);
    return UNITY_END();
}
