#pragma once

#include "../type/season.h"
#include "../type/weather-condition.h"
#include "../type/weather-sprite.h"
#include "pebble.h"

#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
#define WEATHER_ICON_W 36
#define WEATHER_ICON_H 24
#else
#define WEATHER_ICON_W 24
#define WEATHER_ICON_H 16
#endif

void weather_sprites_init(Season season, WeatherCondition condition);
void weather_sprites_deinit(void);
void weather_sprite_alloc(GBitmap **part_bitmap, enum WeatherSprite part);
void update_season_indicator(Season season);
void update_condition_indicator(WeatherCondition condition);
