#pragma once

#include "../sprite/weather-sprites.h"
#include "../type/season.h"
#include "../type/weather-condition.h"
#include "config.h"
#include "pebble.h"

void weather_init(Layer *parent_layer);
void weather_deinit(void);
void weather_set_season(Season season);
void weather_set_condition(WeatherCondition condition);
