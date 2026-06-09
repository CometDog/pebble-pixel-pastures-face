#pragma once

#include "pebble.h"
#include "settings.h"
#include "type/season.h"
#include "type/weather-condition.h"

#if __has_include("mock_private.h")
#include "mock_private.h"

#define MOCK_DATA 1

int mocked_step_count(void);
int mocked_temperature_c(void);
int mocked_temperature_f(void);
WeatherCondition mocked_weather_condition(void);
Season mocked_season(void);
int mocked_sunrise_hour(void);
int mocked_sunset_hour(void);
int mocked_detail_type(void);
int mocked_temperature_unit(void);

#else
#define MOCK_DATA 0
#endif

// Mockable data is always implemented since it should
// return real data if not mocked
time_t mockable_time(void);
int mockable_battery_percent(void);
