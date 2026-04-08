#include "messaging.h"
#include "pebble.h"
#include "settings.h"
#include "sprite/frame-sprites.h"
#include "sprite/weather-sprites.h"
#include "ui/battery.h"
#include "ui/clock-hand.h"
#include "ui/config.h"
#include "ui/detail.h"
#include "ui/frame.h"
#include "ui/time-display.h"
#include "ui/weather.h"

#define BATTERY_ANIMATION_DURATION_MS 300
#define BATTERY_SHOW_TAP_THRESHOLD_MS 3000
#define BATTERY_VISIBILITY_DURATION_MS 3000
#define BATTERY_TAP_DEBOUNCE_MS 400

static Layer *s_background_layer;
static BitmapLayer *s_background_bitmap_layer;
static GBitmap *s_background_bitmap;
static Layer *s_face_content_layer;

static int16_t s_face_y_offset = 0;
static bool s_battery_visible = false;
static PropertyAnimation *s_battery_anim = NULL;
static PropertyAnimation *s_battery_slide_anim = NULL;
static AppTimer *s_battery_hide_timer = NULL;
static AppTimer *s_first_tap_timer = NULL;
static AppTimer *s_debounce_timer = NULL;
static bool s_second_tap_waiting = false;

static void battery_anim_stopped(Animation *anim, bool finished, void *context);
static void battery_slide_anim_stopped(Animation *anim, bool finished, void *context);
static void battery_hide_callback(void *context);

// MARK: Data refresh

static void refresh_all(void)
{
    tm birthday = settings_get_birthday_as_tm_struct();
    time_t now_time = time(NULL);
    tm now = *localtime(&now_time);
    APP_LOG(APP_LOG_LEVEL_INFO, "Current birthday setting: %d-%d", birthday.tm_mon + 1, birthday.tm_mday);
    if (birthday.tm_mday != 0 && birthday.tm_mday == now.tm_mday && birthday.tm_mon == now.tm_mon)
    {
        weather_set_season(BIRTHDAY);
    }
    else
    {
        weather_set_season(settings_get_season());
    }
    weather_set_condition(settings_get_weather_condition());
    clock_hand_set_sun_times(settings_get_sunrise_hour(), settings_get_sunset_hour());

    if (settings_get_detail_type() == 1)
    {
        int unit = settings_get_temperature_unit();
        int temp = (unit == 0) ? settings_get_temperature_c() : settings_get_temperature_f();
        detail_set_temperature(temp, unit == 0);
    }
    else
    {
        detail_set_steps(settings_get_step_count());
    }
}

// MARK: Health handling

#if defined(PBL_HEALTH)
static void health_update_steps(void)
{
    HealthMetric metric = HealthMetricStepCount;
    time_t now = time(NULL);
    time_t start = time_start_of_today();

    HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric, start, now);
    if (mask & HealthServiceAccessibilityMaskAvailable)
    {
        int step_count = health_service_sum_today(metric);
        settings_set_step_count(step_count);
        detail_set_steps(step_count);
    }
}

static void health_handler(HealthEventType event, void *context)
{
    if (event == HealthEventSignificantUpdate || event == HealthEventMovementUpdate)
    {
        health_update_steps();
    }
}
#endif

// MARK: Unobstructed area handling

static void update_face_content_offset(Window *window)
{
    Layer *window_layer = window_get_root_layer(window);
    GRect window_bounds = layer_get_bounds(window_layer);
    GRect unobstructed_bounds = layer_get_unobstructed_bounds(window_layer);

    int16_t obscured_height = window_bounds.size.h - unobstructed_bounds.size.h;
    bool is_obscured = obscured_height > 0;
    int16_t adjusted_shift = obscured_height - WINDOW_REMOVEABLE_Y_PADDING;
    if (adjusted_shift < 0)
    {
        adjusted_shift = 0;
    }

    frame_set_obscured(is_obscured);
    detail_set_obscured(is_obscured);
    s_face_y_offset = -adjusted_shift;
    battery_set_obscured(is_obscured);
    int16_t frame_x = s_battery_visible ? -FRAME_SLIDE_DISTANCE : 0;
    layer_set_frame(s_face_content_layer,
                    GRect(frame_x, s_face_y_offset, PBL_DISPLAY_WIDTH + FRAME_SLIDE_DISTANCE, PBL_DISPLAY_HEIGHT));
    int16_t battery_x = s_battery_visible ? -BATTERY_SLIDE_DISTANCE : 0;
    layer_set_frame(battery_get_layer(),
                    GRect(battery_x, 0, PBL_DISPLAY_WIDTH + BATTERY_SLIDE_DISTANCE, PBL_DISPLAY_HEIGHT));
}

static void unobstructed_did_change(void *context)
{
    update_face_content_offset((Window *)context);
}

static void unobstructed_change(AnimationProgress progress, void *context)
{
    update_face_content_offset((Window *)context);
}

// MARK: Battery slide-in animation

static void battery_anim_stopped(Animation *anim, bool finished, void *context)
{
    property_animation_destroy(s_battery_anim);
    s_battery_anim = NULL;
}

static void battery_slide_anim_stopped(Animation *anim, bool finished, void *context)
{
    property_animation_destroy(s_battery_slide_anim);
    s_battery_slide_anim = NULL;
}

static void start_slide_animation(bool show)
{
    GRect frame_from = GRect(show ? 0 : -FRAME_SLIDE_DISTANCE, s_face_y_offset,
                             PBL_DISPLAY_WIDTH + FRAME_SLIDE_DISTANCE, PBL_DISPLAY_HEIGHT);
    GRect frame_to = GRect(show ? -FRAME_SLIDE_DISTANCE : 0, s_face_y_offset, PBL_DISPLAY_WIDTH + FRAME_SLIDE_DISTANCE,
                           PBL_DISPLAY_HEIGHT);

    if (s_battery_anim)
    {
        animation_unschedule(property_animation_get_animation(s_battery_anim));
    }
    s_battery_anim = property_animation_create_layer_frame(s_face_content_layer, &frame_from, &frame_to);
    Animation *frame_anim = property_animation_get_animation(s_battery_anim);
    animation_set_duration(frame_anim, BATTERY_ANIMATION_DURATION_MS);
    animation_set_curve(frame_anim, AnimationCurveEaseInOut);
    animation_set_handlers(frame_anim, (AnimationHandlers){.stopped = battery_anim_stopped}, NULL);
    animation_schedule(frame_anim);

    GRect battery_from =
        GRect(show ? 0 : -BATTERY_SLIDE_DISTANCE, 0, PBL_DISPLAY_WIDTH + BATTERY_SLIDE_DISTANCE, PBL_DISPLAY_HEIGHT);
    GRect battery_to =
        GRect(show ? -BATTERY_SLIDE_DISTANCE : 0, 0, PBL_DISPLAY_WIDTH + BATTERY_SLIDE_DISTANCE, PBL_DISPLAY_HEIGHT);

    if (s_battery_slide_anim)
    {
        animation_unschedule(property_animation_get_animation(s_battery_slide_anim));
    }
    s_battery_slide_anim = property_animation_create_layer_frame(battery_get_layer(), &battery_from, &battery_to);
    Animation *battery_anim = property_animation_get_animation(s_battery_slide_anim);
    animation_set_duration(battery_anim, BATTERY_ANIMATION_DURATION_MS);
    animation_set_curve(battery_anim, AnimationCurveEaseInOut);
    animation_set_handlers(battery_anim, (AnimationHandlers){.stopped = battery_slide_anim_stopped}, NULL);
    animation_schedule(battery_anim);
}

static void battery_hide_callback(void *context)
{
    s_battery_hide_timer = NULL;
    s_battery_visible = false;
    start_slide_animation(false);
}

static void battery_show(void)
{
    if (s_battery_visible)
    {
        if (s_battery_hide_timer)
        {
            app_timer_reschedule(s_battery_hide_timer, BATTERY_VISIBILITY_DURATION_MS);
        }
        return;
    }
    s_battery_visible = true;
    battery_update_level(battery_state_service_peek().charge_percent);
    start_slide_animation(true);
    s_battery_hide_timer = app_timer_register(BATTERY_VISIBILITY_DURATION_MS, battery_hide_callback, NULL);
}

// MARK: Tap detection

static void tap_threshold_expired(void *context)
{
    s_first_tap_timer = NULL;
    s_second_tap_waiting = false;
}

static void tap_debounce_expired(void *context)
{
    s_debounce_timer = NULL;
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction)
{
    if (s_debounce_timer)
    {
        return;
    }

    if (!s_second_tap_waiting)
    {
        s_second_tap_waiting = true;
        s_first_tap_timer = app_timer_register(BATTERY_SHOW_TAP_THRESHOLD_MS, tap_threshold_expired, NULL);
        s_debounce_timer = app_timer_register(BATTERY_TAP_DEBOUNCE_MS, tap_debounce_expired, NULL);
    }
    else
    {
        app_timer_cancel(s_first_tap_timer);
        s_first_tap_timer = NULL;
        s_second_tap_waiting = false;
        battery_show();
    }
}

// MARK: Tick time

static void global_tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
    clock_hand_tick_handler(tick_time, units_changed);
    time_display_tick_handler(tick_time, units_changed);
}

// MARK: (De)init

void init(Window *window)
{
    messaging_init(refresh_all);

    tick_timer_service_subscribe(MINUTE_UNIT, global_tick_handler);

    Layer *window_layer = window_get_root_layer(window);

    s_background_layer = layer_create(GRect(0, 0, PBL_DISPLAY_WIDTH, PBL_DISPLAY_HEIGHT));
    layer_add_child(window_layer, s_background_layer);

    s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
    s_background_bitmap_layer = bitmap_layer_create(GRect(0, 0, PBL_DISPLAY_WIDTH, PBL_DISPLAY_HEIGHT));
    bitmap_layer_set_bitmap(s_background_bitmap_layer, s_background_bitmap);
    layer_add_child(s_background_layer, bitmap_layer_get_layer(s_background_bitmap_layer));

    s_face_content_layer = layer_create(GRect(0, 0, PBL_DISPLAY_WIDTH + FRAME_SLIDE_DISTANCE, PBL_DISPLAY_HEIGHT));
    layer_add_child(window_layer, s_face_content_layer);

    frame_sprites_init(settings_get_detail_type() == 0);
    weather_sprites_init(settings_get_season(), settings_get_weather_condition());

    // Order matters for layering
    weather_init(s_face_content_layer);
    detail_init(s_face_content_layer);
    frame_init(s_face_content_layer);
    time_display_init(s_face_content_layer);
    clock_hand_init(s_face_content_layer);
    battery_init(window_layer);

#if defined(PBL_HEALTH)
    if (settings_get_detail_type() == 0)
    {
        if (!health_service_events_subscribe(health_handler, NULL))
        {
            APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available, falling back to temperature");
            settings_set_detail_type(1);
        }
    }
#endif

    refresh_all();

    accel_tap_service_subscribe(accel_tap_handler);

    UnobstructedAreaHandlers handlers = {
        .change = unobstructed_change,
        .did_change = unobstructed_did_change,
    };
    unobstructed_area_service_subscribe(handlers, window);
    update_face_content_offset(window);
}

void deinit(Window *window)
{
    accel_tap_service_unsubscribe();

    if (s_first_tap_timer)
    {
        app_timer_cancel(s_first_tap_timer);
    }
    if (s_debounce_timer)
    {
        app_timer_cancel(s_debounce_timer);
    }
    if (s_battery_hide_timer)
    {
        app_timer_cancel(s_battery_hide_timer);
    }
    if (s_battery_anim)
    {
        animation_unschedule(property_animation_get_animation(s_battery_anim));
    }
    if (s_battery_slide_anim)
    {
        animation_unschedule(property_animation_get_animation(s_battery_slide_anim));
    }

#if defined(PBL_HEALTH)
    if (settings_get_detail_type() == 0)
    {
        health_service_events_unsubscribe();
    }
#endif

    messaging_deinit();

    unobstructed_area_service_unsubscribe();
    tick_timer_service_unsubscribe();

    weather_deinit();
    detail_deinit();
    clock_hand_deinit();
    time_display_deinit();
    battery_deinit();
    frame_deinit();

    frame_sprites_deinit();
    weather_sprites_deinit();

    gbitmap_destroy(s_background_bitmap);
    bitmap_layer_destroy(s_background_bitmap_layer);
    layer_destroy(s_face_content_layer);
    layer_destroy(s_background_layer);
    window_destroy(window);
}

int main(void)
{
    Window *window = window_create();
    window_stack_push(window, false);
    init(window);
    app_event_loop();
    deinit(window);
}
