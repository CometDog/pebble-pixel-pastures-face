#include "time-display.h"

#define SHADOW_OFFSET_COUNT 3

static Layer *s_time_display_layer;
static GFont s_time_display_font;

static char s_time_text[16];
static char s_date_text[16];

static const GPoint shadow_offsets[SHADOW_OFFSET_COUNT] = {{-1, 1}, {-2, 1}, {-1, 2}};

static void draw_text_with_shadow(GContext *ctx, const char *text, GFont font, GRect frame, GColor shadow_color,
                                  GColor text_color)
{
    graphics_context_set_text_color(ctx, shadow_color);
    for (int i = 0; i < SHADOW_OFFSET_COUNT; i++)
    {
        GRect shadow_frame = GRect(frame.origin.x + shadow_offsets[i].x, frame.origin.y + shadow_offsets[i].y,
                                   frame.size.w, frame.size.h);
        graphics_draw_text(ctx, text, font, shadow_frame, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter,
                           NULL);
    }

    graphics_context_set_text_color(ctx, text_color);
    graphics_draw_text(ctx, text, font, frame, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

static void time_display_update_proc(Layer *layer, GContext *ctx)
{
    GRect date_frame;
    GRect time_frame;

    get_rect_for_date(&date_frame);
    get_rect_for_time(&time_frame);

#ifdef PBL_COLOR
    GColor shadow_color = GColorRajah;
#else
    GColor shadow_color = GColorWhite;
#endif
    draw_text_with_shadow(ctx, s_date_text, s_time_display_font, date_frame, shadow_color, GColorBlack);
    draw_text_with_shadow(ctx, s_time_text, s_time_display_font, time_frame, shadow_color, GColorBlack);
}

static void set_display_text(char *dest, size_t dest_size, const char *text)
{
    strncpy(dest, text, dest_size);
    dest[dest_size - 1] = '\0';
    layer_mark_dirty(s_time_display_layer);
}

static void clock_set_text_for_time(time_t time)
{
    struct tm *tick_time = localtime(&time);

    static char time_text[] = "00:00 AM";
    static char day_text[] = "MON.";
    static char date_text[] = "00";
    static char full_date_text[] = "MON. 00";

    strftime(time_text, sizeof(time_text), clock_is_24h_style() ? "%k:%M" : "%l:%M %P", tick_time);

    strftime(day_text, sizeof(day_text), "%a", tick_time);
    day_text[3] = '.';
    strftime(date_text, sizeof(date_text), "%d", tick_time);
    snprintf(full_date_text, sizeof(full_date_text), "%s %s", day_text, date_text);

    set_display_text(s_time_text, sizeof(s_time_text), time_text);
    set_display_text(s_date_text, sizeof(s_date_text), full_date_text);
}

void time_display_tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
    if (units_changed & HOUR_UNIT || units_changed & MINUTE_UNIT || units_changed & DAY_UNIT)
    {
        clock_set_text_for_time(mktime(tick_time));
    }
}

void time_display_init(Layer *parent_layer)
{
    s_time_display_font = fonts_load_custom_font(resource_get_handle(TIME_FONT_RESOURCE));

    s_time_display_layer = layer_create(GRect(0, 0, PBL_DISPLAY_WIDTH, PBL_DISPLAY_HEIGHT));
    layer_set_update_proc(s_time_display_layer, time_display_update_proc);
    layer_add_child(parent_layer, s_time_display_layer);

    clock_set_text_for_time(time(NULL));
}

void time_display_deinit(void)
{
    fonts_unload_custom_font(s_time_display_font);
    layer_destroy(s_time_display_layer);
}
