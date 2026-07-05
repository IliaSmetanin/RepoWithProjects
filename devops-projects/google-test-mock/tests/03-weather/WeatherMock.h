#pragma once

#include "json.hpp"
#include "cpr/cpr.h"

#include <fstream>
#include <gmock/gmock.h>
#include <string>
#include <Weather.h>

// struct MyResponse{
//     int32_t status_code;
//     std::string text;
// };

class WeatherMock : public Weather {
public:
    MOCK_METHOD(std::string, GetLocationKey, (const std::string&), (override));
    MOCK_METHOD(cpr::Response, Get, (const std::string&, const cpr::Url&), (override));
};

class WeatherMockTemp : public Weather {
public:
    MOCK_METHOD(float, GetTemperature, (const std::string&), (override));
    MOCK_METHOD(float, GetTomorrowTemperature, (const std::string&), (override));
};

class WeatherMockTest : public ::testing::Test {
public:
    void SetUp() override;
    // void TearDown() override;

    cpr::Response response;
    cpr::Response except_response;
    cpr::Response forecast_response;
};

