#include "detail.h"

#define DIGIT_COUNT 5

typedef enum
{
    NONE = -1,
    CHAR_0,
    CHAR_1,
    CHAR_2,
    CHAR_3,
    CHAR_4,
    CHAR_5,
    CHAR_6,
    CHAR_7,
    CHAR_8,
    CHAR_9,
    CHAR_MINUS,
    CHAR_F,
    CHAR_C,
    TOTAL_CHARS
} Characters;

static Layer *s_detail_layer;
static GBitmap *s_numbers_bitmap;
static GBitmap *s_digit_bitmaps[TOTAL_CHARS];
static GBitmap *s_left_brace_bitmap;
static GBitmap *s_right_brace_bitmap;
static GBitmap *s_middle_board_bitmap;
static bool s_detail_obscured = false;

static Characters s_detail_digits[DIGIT_COUNT] = {NONE, NONE, NONE, NONE, NONE};

static void detail_update_proc(Layer *layer, GContext *ctx)
{
    graphics_context_set_compositing_mode(ctx, GCompOpSet);

    int16_t obscured_extra_offset = s_detail_obscured ? DETAIL_OBSCURED_EXTRA_Y_OFFSET : 0;

    GPoint digit_origin;
    get_origin_for_detail_digits(&digit_origin);
    int x_start = digit_origin.x;
    int y_start = digit_origin.y;

    y_start -= obscured_extra_offset;

    for (int i = 0; i < DIGIT_COUNT; i++)
    {
        if (i == 0)
        {
            GRect left_brace_grect;
            get_rect_for_frame_part(&left_brace_grect, FRAME_SPRITE_BOTTOM_BOARD_LEFT_BRACE, s_left_brace_bitmap);
            left_brace_grect.origin.y -= obscured_extra_offset;
            graphics_draw_bitmap_in_rect(ctx, s_left_brace_bitmap, left_brace_grect);
        }
        else if (i == DIGIT_COUNT - 1)
        {
            GRect right_brace_grect;
            get_rect_for_frame_part(&right_brace_grect, FRAME_SPRITE_BOTTOM_BOARD_RIGHT_BRACE, s_right_brace_bitmap);
            right_brace_grect.origin.y -= obscured_extra_offset;
            graphics_draw_bitmap_in_rect(ctx, s_right_brace_bitmap, right_brace_grect);
        }
        else
        {
            GRect middle_board_grect;
            get_rect_for_frame_part(&middle_board_grect, FRAME_SPRITE_BOTTOM_BOARD_MIDDLE, s_middle_board_bitmap);
            middle_board_grect.origin.x += (i - 1) * middle_board_grect.size.w;
            middle_board_grect.origin.y -= obscured_extra_offset;
            graphics_draw_bitmap_in_rect(ctx, s_middle_board_bitmap, middle_board_grect);
        }

        Characters digit = s_detail_digits[i];
        if (digit == NONE)
        {
            continue;
        }
        GRect dest = GRect(x_start + (i * DETAIL_DIGIT_WIDTH) + (i * DETAIL_DIGIT_SPACE), y_start, DETAIL_DIGIT_WIDTH, DETAIL_DIGIT_HEIGHT);
        graphics_draw_bitmap_in_rect(ctx, s_digit_bitmaps[digit], dest);
    }
}

void detail_set_obscured(bool obscured)
{
    s_detail_obscured = obscured;
    layer_mark_dirty(s_detail_layer);
}

void detail_set_steps(int step_count)
{
    if (step_count == 0)
    {
        for (int i = 0; i < DIGIT_COUNT - 1; i++)
        {
            s_detail_digits[i] = NONE;
        }
        s_detail_digits[DIGIT_COUNT - 1] = CHAR_0;
    }
    else
    {
        for (int i = DIGIT_COUNT - 1; i >= 0; i--)
        {
            if (step_count > 0)
            {
                s_detail_digits[i] = step_count % 10;
                step_count /= 10;
            }
            else
            {
                s_detail_digits[i] = NONE;
            }
        }
    }
    layer_mark_dirty(s_detail_layer);
}

void detail_set_temperature(int temp, bool is_celsius)
{
    int unit_char = is_celsius ? CHAR_C : CHAR_F;

    for (int i = 0; i < DIGIT_COUNT; i++)
    {
        s_detail_digits[i] = NONE;
    }

    bool negative = temp < 0;
    if (negative)
    {
        // Invert the temperature to make it easier to work with when extracting digits, we'll add the minus sign back
        // in at the end
        temp = -temp;
    }

    s_detail_digits[DIGIT_COUNT - 1] = unit_char;

    int pos = DIGIT_COUNT - 2;
    if (temp == 0)
    {
        s_detail_digits[pos] = CHAR_0;
    }
    else
    {
        while (temp > 0 && pos >= 0)
        {
            s_detail_digits[pos] = (Characters)(temp % 10);
            temp /= 10;
            pos--;
        }
        if (negative)
        {
            s_detail_digits[pos] = CHAR_MINUS;
        }
    }

    layer_mark_dirty(s_detail_layer);
}

void detail_init(Layer *parent_layer)
{
    s_detail_layer = layer_create(GRect(0, 0, PBL_DISPLAY_WIDTH, PBL_DISPLAY_HEIGHT));
    layer_set_update_proc(s_detail_layer, detail_update_proc);
    layer_add_child(parent_layer, s_detail_layer);

    frame_sprite_alloc(&s_left_brace_bitmap, FRAME_SPRITE_BOTTOM_BOARD_LEFT_BRACE);
    frame_sprite_alloc(&s_middle_board_bitmap, FRAME_SPRITE_BOTTOM_BOARD_MIDDLE);
    frame_sprite_alloc(&s_right_brace_bitmap, FRAME_SPRITE_BOTTOM_BOARD_RIGHT_BRACE);

    s_numbers_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NUMBERS);
    for (int i = 0; i < TOTAL_CHARS; i++)
    {
        s_digit_bitmaps[i] =
            gbitmap_create_as_sub_bitmap(s_numbers_bitmap, GRect(i * DETAIL_DIGIT_WIDTH, 0, DETAIL_DIGIT_WIDTH, DETAIL_DIGIT_HEIGHT));
#ifdef PBL_COLOR
        gbitmap_get_palette(s_digit_bitmaps[i])[0] = GColorBulgarianRose;
#else
        gbitmap_get_palette(s_digit_bitmaps[i])[0] = GColorBlack;
#endif
    }
}

void detail_deinit(void)
{
    for (int i = 0; i < TOTAL_CHARS; i++)
    {
        if (s_digit_bitmaps[i])
        {
            gbitmap_destroy(s_digit_bitmaps[i]);
        }
    }
    if (s_numbers_bitmap)
    {
        gbitmap_destroy(s_numbers_bitmap);
    }

    layer_destroy(s_detail_layer);
}
