#include "mock.h"

#if MOCK_DATA
time_t mockable_time(void)
{
    return MOCK_TIME;
}

int mockable_battery_percent(void)
{
    return MOCK_BATTERY_PERCENT;
}

int mocked_step_count(void)
{
    return MOCK_STEP_COUNT;
}

int mocked_temperature_c(void)
{
    return MOCK_TEMPERATURE_C;
}

int mocked_temperature_f(void)
{
    return MOCK_TEMPERATURE_F;
}

WeatherCondition mocked_weather_condition(void)
{
    return (WeatherCondition)MOCK_WEATHER_CONDITION;
}

Season mocked_season(void)
{
    return (Season)MOCK_SEASON;
}

int mocked_sunrise_hour(void)
{
    return MOCK_SUNRISE_HOUR;
}

int mocked_sunset_hour(void)
{
    return MOCK_SUNSET_HOUR;
}

int mocked_detail_type(void)
{
    return MOCK_DETAIL_TYPE;
}

int mocked_temperature_unit(void)
{
    return MOCK_TEMPERATURE_UNIT;
}
#else
// Since these are system calls (not through settings)
// we will always call them thus they are "mockable"
// and not "mocked"

time_t mockable_time(void)
{
    return time(NULL);
}

int mockable_battery_percent(void)
{
    return battery_state_service_peek().charge_percent;
}
#endif