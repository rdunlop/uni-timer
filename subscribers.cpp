#include "subscribers.h"

uint8_t num_subscribers = 0;
typedef void(*callback_func)(uint8_t event_type, char *event_data);
#define MAX_SUBSCRIBERS 10
callback_func subscribers[MAX_SUBSCRIBERS];

void notify_subscribers(uint8_t event_type, char *event_data) {
  for (int i = 0; i < num_subscribers; i++) {
    subscribers[i](event_type, event_data);
  }
}

void register_subscriber(void (*event_callback)(uint8_t event_type, char *event_data)) {
  if (num_subscribers >= MAX_SUBSCRIBERS) {
    return;
  }
  subscribers[num_subscribers] = event_callback;
  num_subscribers += 1;
}
