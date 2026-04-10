#pragma once

#include "../sprite/frame-sprites.h"
#include "config.h"
#include "pebble.h"

void battery_init(Layer *parent_layer);
void battery_deinit(void);
void battery_update_level(int percent);
void battery_set_obscured(bool obscured);
void battery_lazy_init(void);
Layer *battery_get_layer(void);
