#pragma once

#include "../sprite/frame-sprites.h"
#include "config.h"
#include "pebble.h"

void frame_init(Layer *parent_layer);
void frame_deinit(void);
void frame_set_obscured(bool obscured);
