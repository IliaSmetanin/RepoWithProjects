#include <fstream>

#include "WeatherMock.h"

void WeatherMockTest::SetUp() {
    std::ifstream current_f("../tests/03-weather/current_moscow.json");
    nlohmann::json current_js = nlohmann::json::parse(current_f);
    std::string current_str = current_js.dump();

    response.status_code = 200;
    response.text = current_str;

    except_response.status_code = 300;
    except_response.text = current_str;

    std::ifstream forecast_f("../tests/03-weather/forecast_moscow.json");
    nlohmann::json forecast_js = nlohmann::json::parse(forecast_f);
    std::string forecast_str = forecast_js.dump();

    forecast_response.status_code = 200;
    forecast_response.text = forecast_str;
}

TEST_F(WeatherMockTest, Get_GetLocationKeyMock) {
    WeatherMock weather;
    // ::testing::NiceMock<WeatherMock> nice_weather;

    EXPECT_CALL(weather, GetLocationKey(::testing::_)).WillRepeatedly(::testing::Return("294021"));
        
    EXPECT_CALL(weather, Get("Moscow", ::testing::_))
        .WillOnce(::testing::Return(except_response))
        .WillRepeatedly(::testing::Return(response));

    EXPECT_THROW(weather.GetResponseForCity("Moscow", cpr::Url{""}), std::invalid_argument);
    ASSERT_EQ(1, 1);

    ASSERT_DOUBLE_EQ(weather.GetTemperature("Moscow"), 0.0);
}

TEST_F(WeatherMockTest, TomorrowTemperatureTest) {
    WeatherMock weather;
    // ::testing::NiceMock<WeatherMock> nice_weather;

    EXPECT_CALL(weather, GetLocationKey(::testing::_)).WillRepeatedly(::testing::Return("294021"));

    EXPECT_CALL(weather, Get("Moscow", ::testing::_))
        .WillRepeatedly(::testing::Return(forecast_response));

    ASSERT_DOUBLE_EQ(weather.GetTomorrowTemperature("Moscow"), 3.2999999523162842);
}

TEST(WeatherMethodsLeft, GetDifferenceStringTest) {
    WeatherMockTemp weather;
    weather.SetApiKey("API_KEY");

    EXPECT_CALL(weather, GetTemperature("Sochi"))
    .Times(2)
    .WillOnce(::testing::Return(10))
    .WillOnce(::testing::Return(15));

    EXPECT_CALL(weather, GetTemperature("Saint-Petersburg"))
    .Times(2)
    .WillOnce(::testing::Return(15))
    .WillOnce(::testing::Return(10));

    ASSERT_EQ(weather.GetDifferenceString("Sochi", "Saint-Petersburg"),
                 "Weather in Sochi is colder than in Saint-Petersburg by 5 degrees");
    ASSERT_EQ(weather.GetDifferenceString("Sochi", "Saint-Petersburg"),
                 "Weather in Sochi is warmer than in Saint-Petersburg by 5 degrees");
}

TEST(WeatherMethodsLeft, GetTomorrowDiffTest) {
    WeatherMockTemp weather;

    EXPECT_CALL(weather, GetTemperature("Orekhovo-Zuyevo"))
    .Times(5)
    .WillOnce(::testing::Return(6.9))
    .WillOnce(::testing::Return(9.4))
    .WillOnce(::testing::Return(13.1))
    .WillOnce(::testing::Return(10.6))
    .WillOnce(::testing::Return(10));

    EXPECT_CALL(weather, GetTomorrowTemperature("Orekhovo-Zuyevo"))
    .WillRepeatedly(::testing::Return(10));

    ASSERT_EQ(weather.GetTomorrowDiff("Orekhovo-Zuyevo"),
              "The weather in Orekhovo-Zuyevo tomorrow will be much warmer than today.");
    ASSERT_EQ(weather.GetTomorrowDiff("Orekhovo-Zuyevo"),
              "The weather in Orekhovo-Zuyevo tomorrow will be warmer than today.");
    ASSERT_EQ(weather.GetTomorrowDiff("Orekhovo-Zuyevo"),
              "The weather in Orekhovo-Zuyevo tomorrow will be much colder than today.");
    ASSERT_EQ(weather.GetTomorrowDiff("Orekhovo-Zuyevo"),
              "The weather in Orekhovo-Zuyevo tomorrow will be colder than today.");
    ASSERT_EQ(weather.GetTomorrowDiff("Orekhovo-Zuyevo"),
              "The weather in Orekhovo-Zuyevo tomorrow will be the same than today.");
}

TEST(WeatherMockLastTest, GetLocationKeyTest) {
    Weather weather;
    EXPECT_THROW(weather.GetLocationKey("Orekhovo-Zuyevo"), std::invalid_argument);
}