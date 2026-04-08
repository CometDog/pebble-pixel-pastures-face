#include "settings.h"

#define SEASON_KEY 1
#define WEATHER_CONDITION_KEY 2
#define SUNRISE_HOUR_KEY 3
#define SUNSET_HOUR_KEY 4
#define STEP_COUNT_KEY 5
#define TEMPERATURE_C_KEY 7
#define TEMPERATURE_F_KEY 8
#define SETTINGS_DETAIL_TYPE_KEY 9
#define SETTINGS_TEMPERATURE_UNIT_KEY 10
#define SETTINGS_BIRTHDAY_KEY 11

static int settings_read(int key, int default_val)
{
    return persist_exists(key) ? persist_read_int(key) : default_val;
}

void settings_set_season(Season season)
{
    persist_write_int(SEASON_KEY, season);
}

Season settings_get_season()
{
    return settings_read(SEASON_KEY, SPRING);
}

void settings_set_weather_condition(WeatherCondition condition)
{
    persist_write_int(WEATHER_CONDITION_KEY, condition);
}

WeatherCondition settings_get_weather_condition()
{
    return settings_read(WEATHER_CONDITION_KEY, RAINY);
}

void settings_set_sunrise_hour(int sunriseHour)
{
    persist_write_int(SUNRISE_HOUR_KEY, sunriseHour);
}

int settings_get_sunrise_hour()
{
    return settings_read(SUNRISE_HOUR_KEY, 0);
}

void settings_set_sunset_hour(int sunsetHour)
{
    persist_write_int(SUNSET_HOUR_KEY, sunsetHour);
}

int settings_get_sunset_hour()
{
    return settings_read(SUNSET_HOUR_KEY, 0);
}

void settings_set_step_count(int stepCount)
{
    persist_write_int(STEP_COUNT_KEY, stepCount);
}

int settings_get_step_count()
{
    return settings_read(STEP_COUNT_KEY, 0);
}

void settings_set_temperature_c(int temperatureC)
{
    persist_write_int(TEMPERATURE_C_KEY, temperatureC);
}

int settings_get_temperature_c()
{
    return settings_read(TEMPERATURE_C_KEY, 0);
}

void settings_set_temperature_f(int temperatureF)
{
    persist_write_int(TEMPERATURE_F_KEY, temperatureF);
}

int settings_get_temperature_f()
{
    return settings_read(TEMPERATURE_F_KEY, 0);
}

void settings_set_detail_type(int detailType)
{
    persist_write_int(SETTINGS_DETAIL_TYPE_KEY, detailType);
}

int settings_get_detail_type()
{
#if !defined(PBL_HEALTH)
    return 1;
#else
    return settings_read(SETTINGS_DETAIL_TYPE_KEY, 0);
#endif
}

void settings_set_temperature_unit(int temperatureUnit)
{
    persist_write_int(SETTINGS_TEMPERATURE_UNIT_KEY, temperatureUnit);
}

int settings_get_temperature_unit()
{
    return settings_read(SETTINGS_TEMPERATURE_UNIT_KEY, 0);
}

void settings_set_birthday(const char *birthday)
{
    persist_write_string(SETTINGS_BIRTHDAY_KEY, birthday);
}

tm settings_get_birthday_as_tm_struct()
{
    if (!persist_exists(SETTINGS_BIRTHDAY_KEY))
    {
        // I can know this is null because day will be 0, something not possible with a valid date
        return (tm){0};
    }
    int max_birthday_str_length = 11; // "YYYY-MM-DD" + null terminator
    char birthday_str[max_birthday_str_length];
    persist_read_string(SETTINGS_BIRTHDAY_KEY, birthday_str, sizeof(birthday_str));
    tm birthday_tm = {0};
    // Parse fixed "YYYY-MM-DD" format using character arithmetic; only necessary because I don't think strptime is
    // available in the Pebble SDK
    int year = (birthday_str[0] - '0') * 1000 + (birthday_str[1] - '0') * 100 + (birthday_str[2] - '0') * 10 +
               (birthday_str[3] - '0');
    int month = (birthday_str[5] - '0') * 10 + (birthday_str[6] - '0');
    int day = (birthday_str[8] - '0') * 10 + (birthday_str[9] - '0');
    birthday_tm.tm_year = year - 1900;
    birthday_tm.tm_mon = month - 1;
    birthday_tm.tm_mday = day;
    return birthday_tm;
}