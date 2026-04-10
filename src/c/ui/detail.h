#pragma once

#include "../sprite/frame-sprites.h"
#include "config.h"
#include "pebble.h"

void detail_init(Layer *parent_layer, bool use_health);
void detail_deinit(void);
void detail_set_mode(bool use_health);
void detail_set_obscured(bool obscured);
void detail_set_steps(int step_count);
void detail_set_temperature(int temp, bool is_celsius);
