#pragma once

#include "config.h"
#include "pebble.h"

void time_display_init(Layer *parent_layer);
void time_display_deinit(void);
void time_display_tick_handler(struct tm *tick_time, TimeUnits units_changed);
