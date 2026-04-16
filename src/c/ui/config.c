#include "config.h"

#if defined(PBL_PLATFORM_GABBRO)
#define LAYOUT_X 23
#define LAYOUT_Y 46
#define LINE_HEIGHT 10
#define LINE_DESCENDER 3

#define TIME_DATE_X_ORIGIN (LAYOUT_X + 71)
#define DATE_Y_ORIGIN (LAYOUT_Y + 10)
#define TIME_DATE_RECT_Y_DISTANCE 65
#define TIME_DATE_RECT_WIDTH 112
#define TIME_DATE_RECT_HEIGHT (28 + LINE_HEIGHT + LINE_DESCENDER)

static const GRect s_clock_hand_rect = {.origin = {LAYOUT_X + 9, LAYOUT_Y + 4}, .size = {106, 121}};
static const GPoint s_time_board_origin = {LAYOUT_X + 0, LAYOUT_Y + 0};
static const GPoint s_bottom_indicator_origin = {LAYOUT_X + 57, LAYOUT_Y + 126};

#define BATTERY_Y_ORIGIN (LAYOUT_Y - 19)
#elif defined(PBL_PLATFORM_CHALK)
#define LAYOUT_X 11
#define LAYOUT_Y 33
#define LINE_HEIGHT 8
#define LINE_DESCENDER 2

#define TIME_DATE_X_ORIGIN (LAYOUT_X + 54)
#define DATE_Y_ORIGIN (LAYOUT_Y + 8)
#define TIME_DATE_RECT_Y_DISTANCE 44
#define TIME_DATE_RECT_WIDTH 80
#define TIME_DATE_RECT_HEIGHT (19 + LINE_HEIGHT + LINE_DESCENDER)

static const GRect s_clock_hand_rect = {.origin = {LAYOUT_X + 9, LAYOUT_Y + 0}, .size = {77, 88}};
static const GPoint s_time_board_origin = {LAYOUT_X + 0, LAYOUT_Y + 0};
static const GPoint s_bottom_indicator_origin = {LAYOUT_X + 50, LAYOUT_Y + 86};

#define BATTERY_Y_ORIGIN (LAYOUT_Y - 11)
#elif defined(PBL_PLATFORM_EMERY)
#define LAYOUT_X 3
#define LAYOUT_Y 34
#define LINE_HEIGHT 10
#define LINE_DESCENDER 3

#define TIME_DATE_X_ORIGIN (LAYOUT_X + 73)
#define DATE_Y_ORIGIN (LAYOUT_Y + 10)
#define TIME_DATE_RECT_Y_DISTANCE 65
#define TIME_DATE_RECT_WIDTH 112
#define TIME_DATE_RECT_HEIGHT (28 + LINE_HEIGHT + LINE_DESCENDER)

static const GRect s_clock_hand_rect = {.origin = {LAYOUT_X + 10, LAYOUT_Y + 4}, .size = {106, 121}};
static const GPoint s_time_board_origin = {LAYOUT_X + 0, LAYOUT_Y + 0};
static const GPoint s_bottom_indicator_origin = {LAYOUT_X + 57, LAYOUT_Y + 126};

#define BATTERY_Y_ORIGIN (LAYOUT_Y - 23)
#else
#define LAYOUT_X 1
#define LAYOUT_Y 27
#define LINE_HEIGHT 8
#define LINE_DESCENDER 2

#define TIME_DATE_X_ORIGIN (LAYOUT_X + 54)
#define DATE_Y_ORIGIN (LAYOUT_Y + 8)
#define TIME_DATE_RECT_Y_DISTANCE 44
#define TIME_DATE_RECT_WIDTH 80
#define TIME_DATE_RECT_HEIGHT (19 + LINE_HEIGHT + LINE_DESCENDER)

static const GRect s_clock_hand_rect = {.origin = {LAYOUT_X + 9, LAYOUT_Y + 2}, .size = {77, 88}};
static const GPoint s_time_board_origin = {LAYOUT_X + 0, LAYOUT_Y + 0};
static const GPoint s_bottom_indicator_origin = {LAYOUT_X + 50, LAYOUT_Y + 86};

#define BATTERY_Y_ORIGIN (LAYOUT_Y - 11)
#endif

static const GRect s_time_rect = {
    .origin = {TIME_DATE_X_ORIGIN, DATE_Y_ORIGIN + TIME_DATE_RECT_Y_DISTANCE - LINE_HEIGHT},
    .size = {TIME_DATE_RECT_WIDTH, TIME_DATE_RECT_HEIGHT}};
static const GRect s_date_rect = {.origin = {TIME_DATE_X_ORIGIN, DATE_Y_ORIGIN - LINE_HEIGHT},
                                  .size = {TIME_DATE_RECT_WIDTH, TIME_DATE_RECT_HEIGHT}};

static const GPoint s_battery_sprite_origins[] = {
    [FRAME_SPRITE_BATTERY_TOP] = {PBL_DISPLAY_WIDTH, BATTERY_Y_ORIGIN},
    [FRAME_SPRITE_BATTERY_BAR_BASE] = {PBL_DISPLAY_WIDTH, BATTERY_Y_ORIGIN + BATTERY_TOP_SPRITE_H},
    [FRAME_SPRITE_BATTERY_BAR_EXTENDED] = {PBL_DISPLAY_WIDTH,
                                           BATTERY_Y_ORIGIN + BATTERY_TOP_SPRITE_H + BATTERY_BAR_BASE_SPRITE_H},
    [FRAME_SPRITE_BATTERY_BOTTOM] = {PBL_DISPLAY_WIDTH, BATTERY_Y_ORIGIN + BATTERY_TOP_SPRITE_H +
                                                            BATTERY_BAR_BASE_SPRITE_H + BATTERY_BAR_EXTENDED_SPRITE_H},
};

void get_rect_for_frame_part(GRect *dest, enum FrameSprite part, GBitmap *part_bitmap)
{
    switch (part)
    {
    case FRAME_SPRITE_MAIN_BOARD:
        dest->origin.x = s_time_board_origin.x + TIME_BOARD_SPRITE_W;
        dest->origin.y = s_time_board_origin.y - MAIN_BOARD_Y_OFFSET;
        break;
    case FRAME_SPRITE_TIME_BOARD:
        dest->origin = s_time_board_origin;
        break;
    case FRAME_SPRITE_BOTTOM_BOARD_INDICATOR:
        dest->origin = s_bottom_indicator_origin;
        break;
    case FRAME_SPRITE_BOTTOM_BOARD_LEFT_BRACE:
        dest->origin.x = s_bottom_indicator_origin.x + BOTTOM_PANEL_INDICATOR_SPRITE_W;
        dest->origin.y = s_bottom_indicator_origin.y - BOTTOM_BRACE_Y_OFFSET;
        break;
    case FRAME_SPRITE_BOTTOM_BOARD_MIDDLE:
        dest->origin.x =
            s_bottom_indicator_origin.x + BOTTOM_PANEL_INDICATOR_SPRITE_W + BOTTOM_PANEL_LEFT_BRACE_SPRITE_W;
        dest->origin.y = s_bottom_indicator_origin.y;
        break;
    case FRAME_SPRITE_BOTTOM_BOARD_RIGHT_BRACE:
        dest->origin.x = s_bottom_indicator_origin.x + BOTTOM_PANEL_INDICATOR_SPRITE_W +
                         BOTTOM_PANEL_LEFT_BRACE_SPRITE_W + BOTTOM_PANEL_MIDDLE_SPRITE_W + BOTTOM_RIGHT_BRACE_X_GAP;
        dest->origin.y = s_bottom_indicator_origin.y - BOTTOM_VERTICAL_INSET;
        break;
    case FRAME_SPRITE_BATTERY_TOP:
    case FRAME_SPRITE_BATTERY_BAR_BASE:
    case FRAME_SPRITE_BATTERY_BAR_EXTENDED:
    case FRAME_SPRITE_BATTERY_BOTTOM:
        dest->origin = s_battery_sprite_origins[part];
        break;
    default:
        break;
    }
    dest->size = gbitmap_get_bounds(part_bitmap).size;
}

void get_rect_for_time(GRect *dest)
{
    *dest = s_time_rect;
}

void get_rect_for_date(GRect *dest)
{
    *dest = s_date_rect;
}

void get_rect_for_clock_hand(GRect *dest)
{
    *dest = s_clock_hand_rect;
}

void get_rect_for_season_indicator(GRect *dest)
{
    int16_t main_board_x = s_time_board_origin.x + TIME_BOARD_SPRITE_W;
    int16_t main_board_y = s_time_board_origin.y - MAIN_BOARD_Y_OFFSET;
    dest->origin.x = main_board_x + SEASON_X_OFFSET;
    dest->origin.y = main_board_y + WEATHER_Y_OFFSET;
    dest->size = GSize(WEATHER_ICON_W, WEATHER_ICON_H);
}

void get_rect_for_condition_indicator(GRect *dest)
{
    int16_t main_board_x = s_time_board_origin.x + TIME_BOARD_SPRITE_W;
    int16_t main_board_y = s_time_board_origin.y - MAIN_BOARD_Y_OFFSET;
    dest->origin.x = main_board_x + CONDITION_X_OFFSET;
    dest->origin.y = main_board_y + WEATHER_Y_OFFSET;
    dest->size = GSize(WEATHER_ICON_W, WEATHER_ICON_H);
}

void get_origin_for_detail_digits(GPoint *dest)
{
    dest->x = s_bottom_indicator_origin.x + BOTTOM_DIGIT_X_OFFSET;
    dest->y = s_bottom_indicator_origin.y + BOTTOM_VERTICAL_INSET;
}
