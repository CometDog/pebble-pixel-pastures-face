#include "messaging.h"

static MessagingCallback s_on_update;

static void inbox_received_handler(DictionaryIterator *iter, void *context)
{
    if (packet_contains_key(iter, MESSAGE_KEY_type))
    {
        if (strcmp(packet_get_string(iter, MESSAGE_KEY_type), "ready") == 0)
        {
            if (packet_begin())
            {
                packet_put_string(MESSAGE_KEY_type, "requestWeatherUpdate");
                packet_send(NULL);
            }
        }
        else if (strcmp(packet_get_string(iter, MESSAGE_KEY_type), "weatherUpdate") == 0)
        {
            APP_LOG(APP_LOG_LEVEL_INFO, "Received weather update!");
            settings_set_season(packet_get_integer(iter, MESSAGE_KEY_season));
            settings_set_weather_condition(packet_get_integer(iter, MESSAGE_KEY_weatherCondition));
            settings_set_sunrise_hour(packet_get_integer(iter, MESSAGE_KEY_sunriseHour));
            settings_set_sunset_hour(packet_get_integer(iter, MESSAGE_KEY_sunsetHour));
            settings_set_temperature_c(packet_get_integer(iter, MESSAGE_KEY_temperatureC));
            settings_set_temperature_f(packet_get_integer(iter, MESSAGE_KEY_temperatureF));
            s_on_update();
        }
    }
    else
    {
        APP_LOG(APP_LOG_LEVEL_INFO, "Received settings update!");
        if (packet_contains_key(iter, MESSAGE_KEY_SETTINGS_DETAIL_TYPE))
        {
            APP_LOG(APP_LOG_LEVEL_INFO, "Updating detail type to %s",
                    packet_get_string(iter, MESSAGE_KEY_SETTINGS_DETAIL_TYPE));
            settings_set_detail_type(atoi(packet_get_string(iter, MESSAGE_KEY_SETTINGS_DETAIL_TYPE)));
        }

        if (packet_contains_key(iter, MESSAGE_KEY_SETTINGS_TEMPERATURE_UNIT))
        {
            APP_LOG(APP_LOG_LEVEL_INFO, "Updating temperature unit to %s",
                    packet_get_string(iter, MESSAGE_KEY_SETTINGS_TEMPERATURE_UNIT));
            settings_set_temperature_unit(atoi(packet_get_string(iter, MESSAGE_KEY_SETTINGS_TEMPERATURE_UNIT)));
        }

        if (packet_contains_key(iter, MESSAGE_KEY_SETTINGS_BIRTHDAY))
        {
            APP_LOG(APP_LOG_LEVEL_INFO, "Updating birthday to %s",
                    packet_get_string(iter, MESSAGE_KEY_SETTINGS_BIRTHDAY));
            settings_set_birthday(packet_get_string(iter, MESSAGE_KEY_SETTINGS_BIRTHDAY));
        }

        s_on_update();
    }
}

void messaging_init(MessagingCallback on_update)
{
    s_on_update = on_update;
    packet_init();
    app_message_register_inbox_received(inbox_received_handler);
    app_message_open(128, 128);
}

void messaging_deinit(void)
{
    app_message_deregister_callbacks();
}
