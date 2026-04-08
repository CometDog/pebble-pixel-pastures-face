#include "frame.h"

static Layer *s_frame_layer;
static GBitmap *s_main_frame_bitmap;
static GBitmap *s_time_display_board_bitmap;
static GBitmap *s_bottom_board_indicator_bitmap;
static BitmapLayer *s_frame_bitmap_layer;
static BitmapLayer *s_bottom_board_indicator_layer;
static GRect s_bottom_board_indicator_rect;

static BitmapLayer *create_sprite_layer(GBitmap *bitmap, GRect rect, Layer *parent)
{
    BitmapLayer *layer = bitmap_layer_create(rect);
    bitmap_layer_set_background_color(layer, GColorClear);
    bitmap_layer_set_compositing_mode(layer, GCompOpSet);
    bitmap_layer_set_bitmap(layer, bitmap);
    layer_add_child(parent, bitmap_layer_get_layer(layer));
    return layer;
}

void frame_init(Layer *parent_layer)
{
    s_frame_layer = layer_create(GRect(0, 0, PBL_DISPLAY_WIDTH, PBL_DISPLAY_HEIGHT));
    layer_add_child(parent_layer, s_frame_layer);

    frame_sprite_alloc(&s_main_frame_bitmap, FRAME_SPRITE_MAIN_BOARD);
    GRect main_frame_rect;
    get_rect_for_frame_part(&main_frame_rect, FRAME_SPRITE_MAIN_BOARD, s_main_frame_bitmap);
    s_frame_bitmap_layer = create_sprite_layer(s_main_frame_bitmap, main_frame_rect, s_frame_layer);

    frame_sprite_alloc(&s_time_display_board_bitmap, FRAME_SPRITE_TIME_BOARD);
    GRect time_board_rect;
    get_rect_for_frame_part(&time_board_rect, FRAME_SPRITE_TIME_BOARD, s_time_display_board_bitmap);
    create_sprite_layer(s_time_display_board_bitmap, time_board_rect, s_frame_layer);

    frame_sprite_alloc(&s_bottom_board_indicator_bitmap, FRAME_SPRITE_BOTTOM_BOARD_INDICATOR);
    get_rect_for_frame_part(&s_bottom_board_indicator_rect, FRAME_SPRITE_BOTTOM_BOARD_INDICATOR,
                            s_bottom_board_indicator_bitmap);
    s_bottom_board_indicator_layer =
        create_sprite_layer(s_bottom_board_indicator_bitmap, s_bottom_board_indicator_rect, s_frame_layer);
}

void frame_set_obscured(bool obscured)
{
    GRect adjusted_rect = s_bottom_board_indicator_rect;
    if (obscured)
    {
        adjusted_rect.origin.y -= FRAME_OBSCURED_EXTRA_Y_OFFSET;
    }
    layer_set_frame(bitmap_layer_get_layer(s_bottom_board_indicator_layer), adjusted_rect);
}

void frame_deinit(void)
{
    bitmap_layer_destroy(s_bottom_board_indicator_layer);
    bitmap_layer_destroy(s_frame_bitmap_layer);
    layer_destroy(s_frame_layer);
}
