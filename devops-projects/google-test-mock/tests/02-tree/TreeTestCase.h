#pragma once

#include <gtest/gtest.h>
#include <filesystem>

class TreeTestCase : public ::testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
};


