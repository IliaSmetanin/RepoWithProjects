#pragma once

#include <gtest/gtest.h>


static const int32_t tricky_year = 1930;
static const int32_t common_year = 1929;
class LeapTestCase : public ::testing::Test {
public:
    int32_t negative = -10;
    int32_t leap1 = 400;
    int32_t leap2 = 2004;
    int32_t common1 = 2100;
    int32_t common2 = 2026;
};

class GetMonthDaysTestCase1 : public ::testing::TestWithParam<int32_t> {
};

class GetMonthDaysTestCase2 : public ::testing::TestWithParam<int32_t> {
};
