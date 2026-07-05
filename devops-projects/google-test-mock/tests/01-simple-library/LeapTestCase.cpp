#include "LeapTestCase.h"

// #include "gtest/gtest.h"
#include <Functions.h>

TEST_F(LeapTestCase, years) {
    EXPECT_THROW(IsLeap(negative), std::invalid_argument);

    ASSERT_TRUE(IsLeap(leap1));
    ASSERT_TRUE(IsLeap(leap2));
    ASSERT_FALSE(IsLeap(common1));
    ASSERT_FALSE(IsLeap(common2));
}

INSTANTIATE_TEST_SUITE_P(
    CheckMonth,
    GetMonthDaysTestCase1,
    ::testing::Values(-1, 0, 13)
);

TEST_P(GetMonthDaysTestCase1, WrongMonth) {
    int32_t m = GetParam();
    EXPECT_THROW(GetMonthDays(common_year, m), std::out_of_range);
}

INSTANTIATE_TEST_SUITE_P(
    LeftMonths,
    GetMonthDaysTestCase2,
    ::testing::Values(4, 6, 9, 11)
);

TEST_P(GetMonthDaysTestCase2, LeastCases) {
    ASSERT_EQ(GetMonthDays(2000, 2), 29);
    ASSERT_EQ(GetMonthDays(common_year, 2), 28);

    ASSERT_EQ(GetMonthDays(tricky_year, 5), 30);
    ASSERT_EQ(GetMonthDays(common_year, 7), 31);

    int32_t m = GetParam();
    ASSERT_EQ(GetMonthDays(common_year, m), 30);
}