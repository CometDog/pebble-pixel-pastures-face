#pragma once

#include "pebble.h"
#include "settings.h"
#include <pebble-packet/pebble-packet.h>

typedef void (*MessagingCallback)(void);
void messaging_init(MessagingCallback on_update);
void messaging_deinit(void);
