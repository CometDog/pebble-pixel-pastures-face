#include "battery.h"

static Layer *s_battery_container;
static BitmapLayer *s_battery_top_layer;
static BitmapLayer *s_battery_bar_layer_1;
static BitmapLayer *s_battery_bar_layer_2;
static BitmapLayer *s_battery_bottom_layer;
static Layer *s_battery_fill_layer;
static GBitmap *s_battery_top_bitmap;
static GBitmap *s_battery_bar_bitmap;
static GBitmap *s_battery_bar_bitmap_2;
static GBitmap *s_battery_bottom_bitmap;
static uint8_t s_battery_percent = 0;
static bool s_obscured = false;
static bool s_created = false;
static int16_t s_bar_1_h = 0;
static int16_t s_bar_2_h = 0;
static GRect s_battery_bar_rect;
static GRect s_battery_top_rect;
static GRect s_battery_bottom_rect;

static BitmapLayer *create_sprite_layer(GBitmap *bitmap, GRect rect, Layer *parent)
{
    BitmapLayer *layer = bitmap_layer_create(rect);
    bitmap_layer_set_background_color(layer, GColorClear);
    bitmap_layer_set_compositing_mode(layer, GCompOpSet);
    bitmap_layer_set_bitmap(layer, bitmap);
    layer_add_child(parent, bitmap_layer_get_layer(layer));
    return layer;
}

static void battery_fill_update_proc(Layer *layer, GContext *ctx)
{
    GRect bounds = layer_get_bounds(layer);
    int16_t total_h = s_obscured ? s_bar_1_h : bounds.size.h;
    int16_t fill_h = total_h * s_battery_percent / 100;
    int16_t y_offset = s_obscured ? (s_bar_1_h - fill_h) : (bounds.size.h - fill_h);
    GRect fill_rect = GRect(BATTERY_BAR_X_PADDING / 2, y_offset, bounds.size.w - BATTERY_BAR_X_PADDING, fill_h);
#ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorGreen);
#else
    graphics_context_set_fill_color(ctx, GColorBlack);
#endif
    graphics_fill_rect(ctx, fill_rect, 0, GCornerNone);
}

void battery_init(Layer *parent_layer)
{
    s_battery_container = layer_create(GRect(0, 0, PBL_DISPLAY_WIDTH + BATTERY_SLIDE_DISTANCE, PBL_DISPLAY_HEIGHT));
    layer_add_child(parent_layer, s_battery_container);
}

void battery_lazy_init(void)
{
    if (s_created)
    {
        return;
    }
    s_created = true;

    frame_sprite_alloc(&s_battery_top_bitmap, FRAME_SPRITE_BATTERY_TOP);
    get_rect_for_frame_part(&s_battery_top_rect, FRAME_SPRITE_BATTERY_TOP, s_battery_top_bitmap);
    s_battery_top_layer = create_sprite_layer(s_battery_top_bitmap, s_battery_top_rect, s_battery_container);

    frame_sprite_alloc(&s_battery_bar_bitmap, FRAME_SPRITE_BATTERY_BAR_BASE);
    get_rect_for_frame_part(&s_battery_bar_rect, FRAME_SPRITE_BATTERY_BAR_BASE, s_battery_bar_bitmap);
    s_bar_1_h = s_battery_bar_rect.size.h;

    s_battery_bar_layer_1 = create_sprite_layer(s_battery_bar_bitmap, s_battery_bar_rect, s_battery_container);

    frame_sprite_alloc(&s_battery_bar_bitmap_2, FRAME_SPRITE_BATTERY_BAR_EXTENDED);
    GRect battery_bar_rect_2;
    get_rect_for_frame_part(&battery_bar_rect_2, FRAME_SPRITE_BATTERY_BAR_EXTENDED, s_battery_bar_bitmap_2);
    s_bar_2_h = battery_bar_rect_2.size.h;
    s_battery_bar_layer_2 = create_sprite_layer(s_battery_bar_bitmap_2, battery_bar_rect_2, s_battery_container);

    frame_sprite_alloc(&s_battery_bottom_bitmap, FRAME_SPRITE_BATTERY_BOTTOM);
    get_rect_for_frame_part(&s_battery_bottom_rect, FRAME_SPRITE_BATTERY_BOTTOM, s_battery_bottom_bitmap);
    s_battery_bottom_layer = create_sprite_layer(s_battery_bottom_bitmap, s_battery_bottom_rect, s_battery_container);

    GRect fill_rect = s_battery_bar_rect;
    fill_rect.size.h = s_bar_1_h + s_bar_2_h + BATTERY_BOTTOM_FILL;
    s_battery_fill_layer = layer_create(fill_rect);
    layer_set_update_proc(s_battery_fill_layer, battery_fill_update_proc);
    layer_add_child(s_battery_container, s_battery_fill_layer);
}

void battery_update_level(int percent)
{
    s_battery_percent = percent;
    if (s_created)
    {
        layer_mark_dirty(s_battery_fill_layer);
    }
}

void battery_set_obscured(bool obscured)
{
    s_obscured = obscured;

    if (!s_created)
    {
        return;
    }

    layer_set_hidden(bitmap_layer_get_layer(s_battery_bar_layer_2), obscured);

    int16_t y_shift = obscured ? (s_battery_top_rect.origin.y - 2) : 0;

    GRect top_rect = s_battery_top_rect;
    top_rect.origin.y -= y_shift;
    layer_set_frame(bitmap_layer_get_layer(s_battery_top_layer), top_rect);

    GRect base_rect = s_battery_bar_rect;
    base_rect.origin.y -= y_shift;
    layer_set_frame(bitmap_layer_get_layer(s_battery_bar_layer_1), base_rect);

    GRect fill_rect = layer_get_frame(s_battery_fill_layer);
    fill_rect.origin.y = base_rect.origin.y;
    layer_set_frame(s_battery_fill_layer, fill_rect);

    GRect bottom_rect = s_battery_bottom_rect;
    if (obscured)
    {
        bottom_rect.origin.y = base_rect.origin.y + s_bar_1_h - BATTERY_BOTTOM_FILL;
    }
    layer_set_frame(bitmap_layer_get_layer(s_battery_bottom_layer), bottom_rect);

    layer_mark_dirty(s_battery_fill_layer);
}

Layer *battery_get_layer(void)
{
    return s_battery_container;
}

void battery_deinit(void)
{
    if (s_created)
    {
        layer_destroy(s_battery_fill_layer);
        bitmap_layer_destroy(s_battery_bottom_layer);
        bitmap_layer_destroy(s_battery_bar_layer_2);
        bitmap_layer_destroy(s_battery_bar_layer_1);
        bitmap_layer_destroy(s_battery_top_layer);
    }
    layer_destroy(s_battery_container);
}
