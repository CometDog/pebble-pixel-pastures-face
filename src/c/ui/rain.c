#include "rain.h"
#include "config.h"

// MARK: Config constants

#define RAIN_DROP_COUNT 35
#define RAIN_FRAME_COUNT 180      // 6 seconds at 30fps
#define RAIN_FRAME_INTERVAL_MS 33 // ~30fps
// The rain lines of stardew valley are about 18 pixels drawn at -115 degrees
// Normally I would scale the rain lines on the larger displays but I prefer the
// smaller sized drops on all platforms so this is a constant on all
#define RAIN_LINE_DX -8 // cos(-115 degrees) * 18 =~ -8
#define RAIN_LINE_DY 16 // sin(-115 degrees) * 18 * -1 =~ 16

#ifdef PBL_COLOR
#define RAIN_DROP_COLOR GColorBabyBlueEyes
#else
#define RAIN_DROP_COLOR GColorWhite
#endif

// MARK: UI and state

static Layer *s_rain_layer;
static bool s_rain_active = false;
static AppTimer *s_rain_timer = NULL;
static int s_rain_frame = 0;

typedef struct
{
    int16_t x;
    int16_t y;
    int8_t speed;
} RainDrop;

static RainDrop s_drops[RAIN_DROP_COUNT];

// MARK: Internal functions

static void rain_stop(void);

static void init_raindrops(void)
{
    // Spread drops evenly across the entire screen with some randomization
    int columns = 7;
    int rows = 5;
    int cell_w = PBL_DISPLAY_WIDTH / columns;
    int cell_h = PBL_DISPLAY_HEIGHT / rows;

    for (int i = 0; i < RAIN_DROP_COUNT; i++)
    {
        int col = i % columns;
        int row = (i / columns) % rows;
        s_drops[i].x = (col * cell_w) + (rand() % cell_w);
        s_drops[i].y = (row * cell_h) + (rand() % cell_h);
        s_drops[i].speed = 2 + (rand() % 4); // 2-5
    }
}

static void draw_rain_layer(Layer *layer, GContext *ctx)
{
    if (!s_rain_active)
    {
        return;
    }

    graphics_context_set_stroke_color(ctx, RAIN_DROP_COLOR);
    graphics_context_set_stroke_width(ctx, 1);

    for (int i = 0; i < RAIN_DROP_COUNT; i++)
    {
        GPoint start = GPoint(s_drops[i].x, s_drops[i].y);
        GPoint end = GPoint(s_drops[i].x + RAIN_LINE_DX, s_drops[i].y + RAIN_LINE_DY);
        graphics_draw_line(ctx, start, end);
    }
}

static void rain_timer_callback(void *data)
{
    s_rain_frame++;
    if (s_rain_frame >= RAIN_FRAME_COUNT)
    {
        rain_stop();
        return;
    }

    for (int i = 0; i < RAIN_DROP_COUNT; i++)
    {
        s_drops[i].x -= s_drops[i].speed;     // moves left
        s_drops[i].y += s_drops[i].speed + 1; // falls down

        if (s_drops[i].y > PBL_DISPLAY_HEIGHT + RAIN_LINE_DY)
        {
            s_drops[i].y = -RAIN_LINE_DY;
            s_drops[i].x = rand() % (PBL_DISPLAY_WIDTH + 20) - 10;
        }
        if (s_drops[i].x < -RAIN_LINE_DX - 10)
        {
            s_drops[i].x = PBL_DISPLAY_WIDTH + 10;
            s_drops[i].y = rand() % PBL_DISPLAY_HEIGHT;
        }
    }

    layer_mark_dirty(s_rain_layer);
    s_rain_timer = app_timer_register(RAIN_FRAME_INTERVAL_MS, rain_timer_callback, NULL);
}

static void rain_stop(void)
{
    s_rain_active = false;
    s_rain_frame = 0;
    if (s_rain_timer)
    {
        app_timer_cancel(s_rain_timer);
        s_rain_timer = NULL;
    }
    layer_mark_dirty(s_rain_layer);
}

// MARK: Public functions

void rain_run(void)
{
    if (s_rain_active)
    {
        rain_stop();
    }

    s_rain_active = true;
    s_rain_frame = 0;
    init_raindrops();

    s_rain_timer = app_timer_register(RAIN_FRAME_INTERVAL_MS, rain_timer_callback, NULL);
    layer_mark_dirty(s_rain_layer);
}

void rain_init(Layer *parent_layer)
{
    s_rain_layer = layer_create(GRect(0, 0, PBL_DISPLAY_WIDTH, PBL_DISPLAY_HEIGHT));
    layer_set_update_proc(s_rain_layer, draw_rain_layer);
    layer_add_child(parent_layer, s_rain_layer);
}

void rain_deinit(void)
{
    rain_stop();
    layer_destroy(s_rain_layer);
}