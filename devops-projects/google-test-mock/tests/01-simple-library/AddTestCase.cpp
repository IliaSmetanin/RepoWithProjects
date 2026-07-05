#include "AddTestCase.h"
#include "Functions.h"

INSTANTIATE_TEST_SUITE_P(
    TestAll,
    AddTestCase,
    ::testing::Values(-1, 3, 0)
);

TEST_P(AddTestCase, CheckSummand) {
    int param = GetParam();
    EXPECT_EQ(Add(param, 1), param + 1);
    EXPECT_EQ(Add(1, param), 1 + param);
}