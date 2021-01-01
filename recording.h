#pragma once

#include "accurate_timing.h"
void store_racer_number(int racer_number);
void clear_racer_number();
int racer_number();

// ****************
void push_racer_number(int racer_number, char *data);
void publish_time_recorded(uint8_t event_type, char *event_data);
