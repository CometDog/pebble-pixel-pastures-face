#include "clock-hand.h"

static Layer *s_clock_hand_layer;
static int8_t s_sunrise_hour = 0;
static int8_t s_sunset_hour = 0;
#ifdef PBL_BW
static GPath *s_clock_hand_path;
#else
static RotBitmapLayer *s_clock_hand_rot_bitmap_layer;
static GBitmap *s_clock_hand_bitmap;
#endif

// 19 (7AM to 1AM) segments divided on a half circle, so 360 / 19 / 2 = 9.47368 degrees or 0.0523599 radians per hour
#define STARTING_HOUR_ANGLE TRIG_MAX_ANGLE / 2 // "south"
#define MAX_ANGLE TRIG_MAX_ANGLE               // "north"
#define HOUR_ANGLE_INCREMENT TRIG_MAX_ANGLE / 19 / 2
#define ENDING_HOUR                                                                                                    \
    25 // 1 AM is the last hour that should be represented on the clock hand, and it should be 180 degrees from the
       // starting hour

// The increments on the semi circle in which the hand should land for sunrise and sunset
#define SUNRISE_INCREMENT 4
#define SUNSET_INCREMENT 10

// BW watches need the hand to be drawn programatically since RotBitmapLayer doesn't rotate 1-bit bitmaps as expected
#ifdef PBL_BW
// TODO: This arrow is poorly drawn and I should do it again
static GPoint s_clock_hand_path_points[] = {
    {1, -36},  {1, -34},  {3, -34},  {3, -30},  {5, -30},  {5, -26},  {7, -26},  {7, -20},
    {3, -20},  {3, 0},    {1, 0},    {1, 2},    {-1, 2},   {-1, 0},   {-3, 0},   {-3, -20},
    {-7, -20}, {-7, -26}, {-5, -26}, {-5, -30}, {-3, -30}, {-3, -34}, {-1, -34}, {-1, -36},
};
static const GPathInfo s_clock_hand_path_info = {
    .num_points = ARRAY_LENGTH(s_clock_hand_path_points),
    .points = s_clock_hand_path_points,
};

static void clock_hand_update_proc(Layer *layer, GContext *ctx)
{
    graphics_context_set_fill_color(ctx, GColorWhite);
    gpath_draw_filled(ctx, s_clock_hand_path);
    graphics_context_set_stroke_color(ctx, GColorBlack);
    gpath_draw_outline(ctx, s_clock_hand_path);
}
#endif

static void clock_hand_set_for_time(time_t time)
{
    struct tm *tick_time = localtime(&time);

    int morning_angle_step = HOUR_ANGLE_INCREMENT;
    int midday_angle_step = HOUR_ANGLE_INCREMENT;
    int evening_angle_step = HOUR_ANGLE_INCREMENT;

    // Morning start hour is adjusted based on the sunrise time and how many increments before sunrise are required, so
    // 7 AM is not guaranteed
    int morning_start_hour = 7;

    // Redeclaring these here because we want to manipulate the sunrise and sunset times to calculate the angle steps,
    // but we don't want to change the actual sunrise and sunset definitions stored globally
    int sunrise = s_sunrise_hour;
    int sunset = s_sunset_hour;
    if (sunrise != 0 && sunset != 0)
    {
        morning_start_hour = sunrise - SUNRISE_INCREMENT;
        if (morning_start_hour < 0)
        {
            morning_start_hour += 24;
        }
        midday_angle_step = (HOUR_ANGLE_INCREMENT * (SUNSET_INCREMENT - SUNRISE_INCREMENT)) / (sunset - sunrise);
        evening_angle_step = (HOUR_ANGLE_INCREMENT * (19 - SUNSET_INCREMENT)) / (ENDING_HOUR - sunset);
    }

    int total_hours = ENDING_HOUR - morning_start_hour;
    int morning_end_hour = (sunrise != 0 && sunset != 0) ? (sunrise - morning_start_hour) : total_hours;
    int midday_end_hour = (sunrise != 0 && sunset != 0) ? (sunset - morning_start_hour) : total_hours;

    // This is the hour at which the clock shifts from "north" to "south" indicating a "new day"
    int mid_night_divide_hour = (ENDING_HOUR % 24 + morning_start_hour) / 2;

    int32_t angle = STARTING_HOUR_ANGLE;

    int hour = tick_time->tm_hour;
    if (hour <= mid_night_divide_hour && hour >= ENDING_HOUR % 24)
    {
        angle = MAX_ANGLE;
    }
    else if (hour > mid_night_divide_hour && hour <= morning_start_hour)
    {
        angle = STARTING_HOUR_ANGLE;
    }
    // The else block here is the actual handling for incrementing the arrow to the correct position based on the
    // sunrise and sunset times, the above if statements are just handling edge cases where the sunrise and sunset times
    // cause the increments to not work as expected
    else
    {
        if (hour < morning_start_hour)
        {
            hour += 24;
        }
        hour -= morning_start_hour;
        if (hour <= morning_end_hour)
        {
            angle += hour * morning_angle_step;
        }
        else if (hour <= midday_end_hour)
        {
            angle += SUNRISE_INCREMENT * morning_angle_step;
            angle += (hour - morning_end_hour) * midday_angle_step;
        }
        else
        {
            angle += SUNRISE_INCREMENT * morning_angle_step;
            angle += (midday_end_hour - morning_end_hour) * midday_angle_step;
            angle += (hour - midday_end_hour) * evening_angle_step;
        }
    }

#ifdef PBL_BW
    gpath_rotate_to(s_clock_hand_path, angle);
    layer_mark_dirty(s_clock_hand_layer);
#else
    rot_bitmap_layer_set_angle(s_clock_hand_rot_bitmap_layer, angle);
#endif
}

void clock_hand_tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
    if (units_changed & HOUR_UNIT || (units_changed & MINUTE_UNIT && tick_time->tm_min == 0))
    {
        clock_hand_set_for_time(mktime(tick_time));
    }
}

void clock_hand_set_sun_times(int sunrise_hour, int sunset_hour)
{
    s_sunrise_hour = sunrise_hour;
    s_sunset_hour = sunset_hour;
    clock_hand_set_for_time(time(NULL));
}

void clock_hand_init(Layer *parent_layer)
{
    GRect clock_hand_frame;
    get_rect_for_clock_hand(&clock_hand_frame);
    s_clock_hand_layer = layer_create(clock_hand_frame);
    layer_add_child(parent_layer, s_clock_hand_layer);

#ifdef PBL_BW
    s_clock_hand_path = gpath_create(&s_clock_hand_path_info);
    GRect bounds = layer_get_bounds(s_clock_hand_layer);
    gpath_move_to(s_clock_hand_path, GPoint((bounds.size.w / 2) - 2, (bounds.size.h / 2) - 8));
    layer_set_update_proc(s_clock_hand_layer, clock_hand_update_proc);
#else
    frame_sprite_alloc(&s_clock_hand_bitmap, FRAME_SPRITE_TIME_ARROW);
    GRect bitmap_bounds = gbitmap_get_bounds(s_clock_hand_bitmap);
    s_clock_hand_rot_bitmap_layer = rot_bitmap_layer_create(s_clock_hand_bitmap);
    rot_bitmap_set_src_ic(s_clock_hand_rot_bitmap_layer, GPoint(bitmap_bounds.size.w / 2, bitmap_bounds.size.h - 2));
    rot_bitmap_set_compositing_mode(s_clock_hand_rot_bitmap_layer, GCompOpSet);
    rot_bitmap_layer_set_angle(s_clock_hand_rot_bitmap_layer, MAX_ANGLE);
    layer_add_child(s_clock_hand_layer, bitmap_layer_get_layer((BitmapLayer *)s_clock_hand_rot_bitmap_layer));
#endif

    clock_hand_set_for_time(time(NULL));
}

void clock_hand_deinit(void)
{
#ifdef PBL_BW
    gpath_destroy(s_clock_hand_path);
#else
    rot_bitmap_layer_destroy(s_clock_hand_rot_bitmap_layer);
#endif
    layer_destroy(s_clock_hand_layer);
}
