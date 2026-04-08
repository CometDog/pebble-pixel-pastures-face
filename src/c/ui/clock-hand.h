#pragma once

#include "../sprite/frame-sprites.h"
#include "config.h"
#include "pebble.h"

void clock_hand_init(Layer *parent_layer);
void clock_hand_deinit(void);
void clock_hand_tick_handler(struct tm *tick_time, TimeUnits units_changed);
void clock_hand_set_sun_times(int sunrise_hour, int sunset_hour);
