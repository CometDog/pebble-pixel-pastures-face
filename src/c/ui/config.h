#pragma once

#include "../sprite/frame-sprites.h"
#include "../sprite/weather-sprites.h"
#include "pebble.h"

#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
#define WINDOW_REMOVEABLE_Y_PADDING 30
#ifdef PBL_PLATFORM_GABBRO
#define FRAME_SLIDE_DISTANCE 56
#define BATTERY_SLIDE_DISTANCE 96
#else
#define FRAME_SLIDE_DISTANCE 42
#define BATTERY_SLIDE_DISTANCE 42
#endif
#define DETAIL_DIGIT_WIDTH 15
#define DETAIL_DIGIT_HEIGHT 21
#define DETAIL_DIGIT_SPACE 3
#define FRAME_OBSCURED_EXTRA_Y_OFFSET 8
#define BATTERY_BOTTOM_FILL 3
#define BATTERY_BAR_X_PADDING 18
#define TIME_FONT_RESOURCE RESOURCE_ID_FONT_SV_REGULAR_32

// Offset from sprite anchors
#define MAIN_BOARD_Y_OFFSET 3
#define WEATHER_Y_OFFSET 45
#define BOTTOM_BRACE_Y_OFFSET 15
#define BOTTOM_VERTICAL_INSET 12
#define BOTTOM_RIGHT_BRACE_X_GAP 36
#define BOTTOM_DIGIT_X_OFFSET 30
#define SEASON_X_OFFSET 78
#define CONDITION_X_OFFSET 12
#else
#define WINDOW_REMOVEABLE_Y_PADDING 28
#ifdef PBL_PLATFORM_CHALK
#define FRAME_SLIDE_DISTANCE 38
#define BATTERY_SLIDE_DISTANCE 62
#else
#define FRAME_SLIDE_DISTANCE 28
#define BATTERY_SLIDE_DISTANCE 28
#endif
#define DETAIL_DIGIT_WIDTH 10
#define DETAIL_DIGIT_HEIGHT 14
#define DETAIL_DIGIT_SPACE 2
#define FRAME_OBSCURED_EXTRA_Y_OFFSET 6
#define BATTERY_BOTTOM_FILL 2
#define BATTERY_BAR_X_PADDING 12
#define TIME_FONT_RESOURCE RESOURCE_ID_FONT_SV_REGULAR_24

// Offset from sprite anchors
#define MAIN_BOARD_Y_OFFSET 2
#define WEATHER_Y_OFFSET 32
#define BOTTOM_BRACE_Y_OFFSET 10
#define BOTTOM_VERTICAL_INSET 8
#define BOTTOM_RIGHT_BRACE_X_GAP 24
#define BOTTOM_DIGIT_X_OFFSET 20
#define SEASON_X_OFFSET 58
#define CONDITION_X_OFFSET 10
#endif

#ifdef PBL_PLATFORM_EMERY
#define DETAIL_OBSCURED_EXTRA_Y_OFFSET 8
#else
#define DETAIL_OBSCURED_EXTRA_Y_OFFSET 6
#endif

void get_rect_for_frame_part(GRect *dest, enum FrameSprite part, GBitmap *part_bitmap);
void get_rect_for_time(GRect *dest);
void get_rect_for_date(GRect *dest);
void get_rect_for_clock_hand(GRect *dest);
void get_rect_for_season_indicator(GRect *dest);
void get_rect_for_condition_indicator(GRect *dest);
void get_origin_for_detail_digits(GPoint *dest);
