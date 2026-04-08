#pragma once

#include "pebble.h"
#include "type/season.h"
#include "type/weather-condition.h"

void settings_set_season(Season season);
Season settings_get_season();

void settings_set_weather_condition(WeatherCondition condition);
WeatherCondition settings_get_weather_condition();

void settings_set_sunrise_hour(int sunriseHour);
int settings_get_sunrise_hour();

void settings_set_sunset_hour(int sunsetHour);
int settings_get_sunset_hour();

void settings_set_step_count(int stepCount);
int settings_get_step_count();

void settings_set_temperature_c(int temperatureC);
int settings_get_temperature_c();

void settings_set_temperature_f(int temperatureF);
int settings_get_temperature_f();

void settings_set_detail_type(int detailType);
int settings_get_detail_type();

void settings_set_temperature_unit(int temperatureUnit);
int settings_get_temperature_unit();

void settings_set_birthday(const char *birthday);
tm settings_get_birthday_as_tm_struct();