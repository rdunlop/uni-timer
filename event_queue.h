#pragma once

#include <Arduino.h>

// EVENTS
#define EVT_BUZZER_CHANGE 0
#define EVT_TIME_CHANGE 1
#define EVT_MODE_CHANGE 2
#define EVT_SENSOR_CHANGE 3

// Useful constants
#define EVENT_STACK_SIZE 10
#define EVT_MAX_STR_LEN 20

// methods
void push_event(uint8_t evt, const char *val);

// shared data
bool pop_event(uint8_t *evt, char *val);
