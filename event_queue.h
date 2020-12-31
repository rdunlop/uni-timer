#pragma once

#include <Arduino.h>

// EVENTS
#define EVT_BUZZER_CHANGE 0
#define EVT_TIME_CHANGE 1
#define EVT_MODE_CHANGE 2
#define EVT_SENSOR_CHANGE 3
#define EVT_SENSOR_BLOCKED 4
#define EVT_TIME_RECORDED 5
#define EVT_TIME_STORED 6
#define EVT_DUPLICATE_RESULT 7
#define EVT_DELETE_RESULT 8
#define EVT_RACER_NUMBER_ENTERED 9
#define EVT_TIMER_COUNTDOWN_FINISHED 10

// Useful constants
#define EVENT_STACK_SIZE 10
#define EVT_MAX_STR_LEN 30

// methods
void push_event(uint8_t evt, const char *val);

// shared data
bool pop_event(uint8_t *evt, char *val);
