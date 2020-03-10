#include "subscribers.h"

uint8_t num_subscribers = 0;
typedef void(*callback_func)(uint8_t event_type, char *event_data);
callback_func subscribers[10];

void notify_subscribers(uint8_t event_type, char *event_data) {
  for (int i = 0; i < num_subscribers; i++) {
    subscribers[i](event_type, event_data);
  }
}

void register_subscriber(void (*event_callback)(uint8_t event_type, char *event_data)) {
  subscribers[num_subscribers] = event_callback;
  num_subscribers += 1;
}
