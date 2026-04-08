#include "weather-sprites.h"

static GBitmap *s_weather_sprites_parent_bitmap;
static GBitmap *s_weather_sprite_bitmaps[WEATHER_SPRITE_COUNT];

#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
#define SPRITE_GAP 3
#else
#define SPRITE_GAP 2
#endif

static const GRect s_season_coords[] = {
    [SPRING] = {.origin = {0, 0}, .size = {WEATHER_ICON_W, WEATHER_ICON_H}},
    [SUMMER] = {.origin = {WEATHER_ICON_W + SPRITE_GAP, 0}, .size = {WEATHER_ICON_W, WEATHER_ICON_H}},
    [FALL] = {.origin = {(WEATHER_ICON_W + SPRITE_GAP) * 2, 0}, .size = {WEATHER_ICON_W, WEATHER_ICON_H}},
    [WINTER] = {.origin = {(WEATHER_ICON_W + SPRITE_GAP) * 3, 0}, .size = {WEATHER_ICON_W, WEATHER_ICON_H}},
    [BIRTHDAY] = {.origin = {WEATHER_ICON_W + SPRITE_GAP, WEATHER_ICON_H + SPRITE_GAP},
                  .size = {WEATHER_ICON_W, WEATHER_ICON_H}},
};

static const GRect s_condition_coords[] = {
    [RAINY] = {.origin = {(WEATHER_ICON_W + SPRITE_GAP) * 2, WEATHER_ICON_H + SPRITE_GAP},
               .size = {WEATHER_ICON_W, WEATHER_ICON_H}},
    [SUNNY] = {.origin = {(WEATHER_ICON_W + SPRITE_GAP) * 3, WEATHER_ICON_H + SPRITE_GAP},
               .size = {WEATHER_ICON_W, WEATHER_ICON_H}},
    [SNOWY] = {.origin = {WEATHER_ICON_W + SPRITE_GAP, (WEATHER_ICON_H + SPRITE_GAP) * 2},
               .size = {WEATHER_ICON_W, WEATHER_ICON_H}},
    [WINDY] = {.origin = {(WEATHER_ICON_W + SPRITE_GAP) * 2, (WEATHER_ICON_H + SPRITE_GAP) * 2},
               .size = {WEATHER_ICON_W, WEATHER_ICON_H}},
    [STORMY] = {.origin = {(WEATHER_ICON_W + SPRITE_GAP) * 3, (WEATHER_ICON_H + SPRITE_GAP) * 2},
                .size = {WEATHER_ICON_W, WEATHER_ICON_H}},
};

void weather_sprites_init(Season season, WeatherCondition condition)
{
    s_weather_sprites_parent_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WEATHER);
    update_season_indicator(season);
    update_condition_indicator(condition);
}

void weather_sprites_deinit(void)
{
    for (int i = 0; i < WEATHER_SPRITE_COUNT; i++)
    {
        gbitmap_destroy(s_weather_sprite_bitmaps[i]);
    }
    gbitmap_destroy(s_weather_sprites_parent_bitmap);
}

void weather_sprite_alloc(GBitmap **part_bitmap, enum WeatherSprite part)
{
    *part_bitmap = s_weather_sprite_bitmaps[part];
}

void update_season_indicator(Season season)
{
    GBitmap *old = s_weather_sprite_bitmaps[WEATHER_SPRITE_SEASON];
    s_weather_sprite_bitmaps[WEATHER_SPRITE_SEASON] =
        gbitmap_create_as_sub_bitmap(s_weather_sprites_parent_bitmap, s_season_coords[season]);
    if (old)
    {
        gbitmap_destroy(old);
    }
}

void update_condition_indicator(WeatherCondition condition)
{
    GBitmap *old = s_weather_sprite_bitmaps[WEATHER_SPRITE_CONDITION];
    s_weather_sprite_bitmaps[WEATHER_SPRITE_CONDITION] =
        gbitmap_create_as_sub_bitmap(s_weather_sprites_parent_bitmap, s_condition_coords[condition]);
    if (old)
    {
        gbitmap_destroy(old);
    }
}
