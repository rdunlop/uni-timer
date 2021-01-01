#pragma once

#include <Arduino.h>

// EVENTS
// The buzzer has changed from on to off, or off to on
#define EVT_BUZZER_CHANGE 0
// The Time has changed (at most, 1/second) (data includes current time)
#define EVT_TIME_CHANGE 1
// The mode is being changed
#define EVT_MODE_CHANGE 2
// he sensor was blocked, or un-blocked # SHOULD THIS BE EVERYWHERE?
#define EVT_SENSOR_CHANGE 3
// The Sensor was blocked (data includes precise time of blockage)
#define EVT_SENSOR_BLOCKED 4
// A time record is generated, store it to SD
#define EVT_TIME_RECORD 5
// A Time record has been stored to SD
#define EVT_TIME_STORED 6
#define EVT_DUPLICATE_RESULT 7
#define EVT_DELETE_RESULT 8
// The racer number is entered. This may trigger the beginning of a countdown (mode 4), or may record a result (mode 6).
#define EVT_RACER_NUMBER_ENTERED 9
// The countdown has finished (data includes precise time of end of countdown)
#define EVT_TIMER_COUNTDOWN_FINISHED 10
// how many time events are cached, awaiting racer number (data will be the count)
#define EVT_CACHED_TIME_COUNT 11
// we have cleared the racer number
#define EVT_RACER_NUMBER_CLEARED 12

// Useful constants
#define EVENT_STACK_SIZE 10
#define EVT_MAX_STR_LEN 30

// methods
void push_event(uint8_t evt, const char *val);

// shared data
bool pop_event(uint8_t *evt, char *val);
