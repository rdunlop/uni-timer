#include "event_queue.h"
#include <Arduino.h>

// EVENT QUEUE MANAGEMENT
uint8_t event_count = 0;
uint8_t next_event = 0;
uint8_t event_type[EVENT_STACK_SIZE];
char event_data[EVENT_STACK_SIZE][EVT_MAX_STR_LEN];

void push_event(uint8_t evt, const char *val) {
//  Serial.print("Pushing event ");
//  Serial.print(evt);
//  Serial.print(": ");
//  Serial.println(val);
  uint8_t next_opening = (next_event + event_count) % EVENT_STACK_SIZE;
  event_type[next_opening] = evt;
  strncpy(event_data[next_opening], val, EVT_MAX_STR_LEN);
  event_count += 1;
}

bool pop_event(uint8_t *evt, char *val) {
  if (event_count == 0) {
    return false;
  }
  *evt = event_type[next_event];
  strncpy(val, event_data[next_event], EVT_MAX_STR_LEN);
  next_event = (next_event + 1) % EVENT_STACK_SIZE;
  event_count -= 1;

  return true;
}
