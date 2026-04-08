#include "weather.h"

static Layer *s_season_layer;
static Layer *s_conditions_layer;
static GBitmap *s_season_bitmap;
static GBitmap *s_conditions_bitmap;

static void draw_seasons_layer(Layer *layer, GContext *ctx)
{
    if (s_season_bitmap)
    {
        GRect frame;
        get_rect_for_season_indicator(&frame);
        graphics_draw_bitmap_in_rect(ctx, s_season_bitmap, frame);
    }
}

static void draw_conditions_layer(Layer *layer, GContext *ctx)
{
    if (s_conditions_bitmap)
    {
        GRect frame;
        get_rect_for_condition_indicator(&frame);
        graphics_draw_bitmap_in_rect(ctx, s_conditions_bitmap, frame);
    }
}

void weather_set_season(Season season)
{
    update_season_indicator(season);
    weather_sprite_alloc(&s_season_bitmap, WEATHER_SPRITE_SEASON);
    layer_mark_dirty(s_season_layer);
}

void weather_set_condition(WeatherCondition condition)
{
    update_condition_indicator(condition);
    weather_sprite_alloc(&s_conditions_bitmap, WEATHER_SPRITE_CONDITION);
    layer_mark_dirty(s_conditions_layer);
}

void weather_init(Layer *parent_layer)
{
    weather_sprite_alloc(&s_season_bitmap, WEATHER_SPRITE_SEASON);
    weather_sprite_alloc(&s_conditions_bitmap, WEATHER_SPRITE_CONDITION);

    s_season_layer = layer_create(GRect(0, 0, PBL_DISPLAY_WIDTH, PBL_DISPLAY_HEIGHT));
    layer_set_update_proc(s_season_layer, draw_seasons_layer);
    layer_add_child(parent_layer, s_season_layer);

    s_conditions_layer = layer_create(GRect(0, 0, PBL_DISPLAY_WIDTH, PBL_DISPLAY_HEIGHT));
    layer_set_update_proc(s_conditions_layer, draw_conditions_layer);
    layer_add_child(parent_layer, s_conditions_layer);
}

void weather_deinit(void)
{
    layer_destroy(s_conditions_layer);
    layer_destroy(s_season_layer);
}
