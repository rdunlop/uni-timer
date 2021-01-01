#pragma once

#include <Arduino.h>

void notify_subscribers(uint8_t event_type, char *event_data);
void register_subscriber(void (*event_callback)(uint8_t event_type, char *event_data));
void unregister_subscriber(void (*event_callback)(uint8_t event_type, char *event_data));
