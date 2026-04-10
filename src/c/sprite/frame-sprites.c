#include "frame-sprites.h"

static GBitmap *s_frame_sprites_bitmap;
static GBitmap *s_frame_sprite_bitmaps[FRAME_SPRITE_COUNT];
static bool s_battery_sprites_created = false;

#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
static const GRect s_frame_sprite_main_board_coords = {.origin = {69, 0},
                                                       .size = {MAIN_BOARD_SPRITE_W, MAIN_BOARD_SPRITE_H}};
static const GRect s_frame_sprite_time_board_coords = {.origin = {0, 0},
                                                       .size = {TIME_BOARD_SPRITE_W, TIME_BOARD_SPRITE_H}};
static const GRect s_frame_sprite_bottom_board_indicator_steps_coords = {
    .origin = {201, 0}, .size = {BOTTOM_PANEL_INDICATOR_SPRITE_W, BOTTOM_PANEL_INDICATOR_SPRITE_H}};
static const GRect s_frame_sprite_bottom_board_indicator_temperature_coords = {
    .origin = {228, 0}, .size = {BOTTOM_PANEL_INDICATOR_SPRITE_W, BOTTOM_PANEL_INDICATOR_SPRITE_H}};
static const GRect s_frame_sprite_bottom_board_left_brace_coords = {
    .origin = {201, 48}, .size = {BOTTOM_PANEL_LEFT_BRACE_SPRITE_W, BOTTOM_PANNEL_LEFT_BRACE_SPRITE_H}};
static const GRect s_frame_sprite_bottom_board_middle_coords = {
    .origin = {255, 0}, .size = {BOTTOM_PANEL_MIDDLE_SPRITE_W, BOTTOM_PANEL_MIDDLE_SPRITE_H}};
static const GRect s_frame_sprite_bottom_board_right_brace_coords = {
    .origin = {226, 48}, .size = {BOTTOM_PANEL_RIGHT_BRACE_SPRITE_W, BOTTOM_PANEL_RIGHT_BRACE_SPRITE_H}};
static const GRect s_frame_sprite_battery_top_coords = {.origin = {267, 78},
                                                        .size = {BATTERY_TOP_SPRITE_W, BATTERY_TOP_SPRITE_H}};
static const GRect s_frame_sprite_battery_bottom_coords = {.origin = {225, 108},
                                                           .size = {BATTERY_BOTTOM_SPRITE_W, BATTERY_BOTTOM_SPRITE_H}};
static const GRect s_frame_sprite_battery_bar_base_coords = {.origin = {318, 0},
                                                             .size = {BATTERY_BAR_SPRITE_W, BATTERY_BAR_BASE_SPRITE_H}};
static const GRect s_frame_sprite_battery_bar_extended_coords = {
    .origin = {276, 0}, .size = {BATTERY_BAR_SPRITE_W, BATTERY_BAR_EXTENDED_SPRITE_H}};
#else
static const GRect s_frame_sprite_main_board_coords = {.origin = {50, 0},
                                                       .size = {MAIN_BOARD_SPRITE_W, MAIN_BOARD_SPRITE_H}};
static const GRect s_frame_sprite_time_board_coords = {.origin = {0, 0},
                                                       .size = {TIME_BOARD_SPRITE_W, TIME_BOARD_SPRITE_H}};
static const GRect s_frame_sprite_bottom_board_indicator_steps_coords = {
    .origin = {146, 0}, .size = {BOTTOM_PANEL_INDICATOR_SPRITE_W, BOTTOM_PANEL_INDICATOR_SPRITE_H}};
static const GRect s_frame_sprite_bottom_board_indicator_temperature_coords = {
    .origin = {164, 0}, .size = {BOTTOM_PANEL_INDICATOR_SPRITE_W, BOTTOM_PANEL_INDICATOR_SPRITE_H}};
static const GRect s_frame_sprite_bottom_board_left_brace_coords = {
    .origin = {146, 32}, .size = {BOTTOM_PANEL_LEFT_BRACE_SPRITE_W, BOTTOM_PANNEL_LEFT_BRACE_SPRITE_H}};
static const GRect s_frame_sprite_bottom_board_middle_coords = {
    .origin = {182, 0}, .size = {BOTTOM_PANEL_MIDDLE_SPRITE_W, BOTTOM_PANEL_MIDDLE_SPRITE_H}};
static const GRect s_frame_sprite_bottom_board_right_brace_coords = {
    .origin = {163, 32}, .size = {BOTTOM_PANEL_RIGHT_BRACE_SPRITE_W, BOTTOM_PANEL_RIGHT_BRACE_SPRITE_H}};
static const GRect s_frame_sprite_battery_top_coords = {.origin = {190, 50},
                                                        .size = {BATTERY_TOP_SPRITE_W, BATTERY_TOP_SPRITE_H}};
static const GRect s_frame_sprite_battery_bottom_coords = {.origin = {162, 72},
                                                           .size = {BATTERY_BOTTOM_SPRITE_W, BATTERY_BOTTOM_SPRITE_H}};
static const GRect s_frame_sprite_battery_bar_base_coords = {.origin = {224, 0},
                                                             .size = {BATTERY_BAR_SPRITE_W, BATTERY_BAR_BASE_SPRITE_H}};
static const GRect s_frame_sprite_battery_bar_extended_coords = {
    .origin = {196, 0}, .size = {BATTERY_BAR_SPRITE_W, BATTERY_BAR_EXTENDED_SPRITE_H}};
#endif

void frame_sprites_init(bool use_health)
{
    s_frame_sprites_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CLOCK_FRAME_PARTS);
    for (int i = 0; i < FRAME_SPRITE_COUNT; i++)
    {
        GRect part_coords;
        switch (i)
        {
        case FRAME_SPRITE_MAIN_BOARD:
            part_coords = s_frame_sprite_main_board_coords;
            break;
        case FRAME_SPRITE_TIME_BOARD:
            part_coords = s_frame_sprite_time_board_coords;
            break;
        case FRAME_SPRITE_BOTTOM_BOARD_INDICATOR:
            part_coords = use_health ? s_frame_sprite_bottom_board_indicator_steps_coords
                                     : s_frame_sprite_bottom_board_indicator_temperature_coords;
            break;
        case FRAME_SPRITE_BOTTOM_BOARD_LEFT_BRACE:
            part_coords = s_frame_sprite_bottom_board_left_brace_coords;
            break;
        case FRAME_SPRITE_BOTTOM_BOARD_MIDDLE:
            part_coords = s_frame_sprite_bottom_board_middle_coords;
            break;
        case FRAME_SPRITE_BOTTOM_BOARD_RIGHT_BRACE:
            part_coords = s_frame_sprite_bottom_board_right_brace_coords;
            break;
        case FRAME_SPRITE_BATTERY_TOP:
        case FRAME_SPRITE_BATTERY_BOTTOM:
        case FRAME_SPRITE_BATTERY_BAR_BASE:
        case FRAME_SPRITE_BATTERY_BAR_EXTENDED:
            continue;
        case FRAME_SPRITE_TIME_ARROW:
            s_frame_sprite_bitmaps[i] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CLOCK_ARROW);
            continue;
        }
        s_frame_sprite_bitmaps[i] = gbitmap_create_as_sub_bitmap(s_frame_sprites_bitmap, part_coords);
    }
}

void frame_sprites_lazy_battery_init(void)
{
    if (s_battery_sprites_created)
    {
        return;
    }
    s_battery_sprites_created = true;

    s_frame_sprite_bitmaps[FRAME_SPRITE_BATTERY_TOP] =
        gbitmap_create_as_sub_bitmap(s_frame_sprites_bitmap, s_frame_sprite_battery_top_coords);
    s_frame_sprite_bitmaps[FRAME_SPRITE_BATTERY_BOTTOM] =
        gbitmap_create_as_sub_bitmap(s_frame_sprites_bitmap, s_frame_sprite_battery_bottom_coords);
    s_frame_sprite_bitmaps[FRAME_SPRITE_BATTERY_BAR_BASE] =
        gbitmap_create_as_sub_bitmap(s_frame_sprites_bitmap, s_frame_sprite_battery_bar_base_coords);
    s_frame_sprite_bitmaps[FRAME_SPRITE_BATTERY_BAR_EXTENDED] =
        gbitmap_create_as_sub_bitmap(s_frame_sprites_bitmap, s_frame_sprite_battery_bar_extended_coords);
}

void frame_sprites_update_indicator(bool use_health)
{
    gbitmap_destroy(s_frame_sprite_bitmaps[FRAME_SPRITE_BOTTOM_BOARD_INDICATOR]);
    GRect coords = use_health ? s_frame_sprite_bottom_board_indicator_steps_coords
                              : s_frame_sprite_bottom_board_indicator_temperature_coords;
    s_frame_sprite_bitmaps[FRAME_SPRITE_BOTTOM_BOARD_INDICATOR] =
        gbitmap_create_as_sub_bitmap(s_frame_sprites_bitmap, coords);
}

void frame_sprites_deinit(void)
{
    for (int i = 0; i < FRAME_SPRITE_COUNT; i++)
    {
        gbitmap_destroy(s_frame_sprite_bitmaps[i]);
    }
    gbitmap_destroy(s_frame_sprites_bitmap);
}

void frame_sprite_alloc(GBitmap **part_bitmap, enum FrameSprite part)
{
    *part_bitmap = s_frame_sprite_bitmaps[part];
}
