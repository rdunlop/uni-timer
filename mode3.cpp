#include "uni_sensor.h"
#include "uni_buzzer.h"
#include "modes.h"
#include "event_queue.h"

extern UniBuzzer buzzer;
extern UniSensor sensor;

//### Mode 3 - Sensor Tuning
//
//- When the Sensor beam is crossed, no noise. When the sensor is not-crossed, beep continuously.

void mode3_setup() {
  buzzer.clear();
}

void mode3_event_handler(uint8_t event_type, char *event_data) {
  switch(event_type) {
    case EVT_SENSOR_CHANGE:
      if (strcmp(event_data, "1")) {
        // sensor crossed
        buzzer.clear();
      } else {
        buzzer.beep(60 * 60 * 1000); // 60 minutes
      }
      break;
  }
}

void mode3_teardown() {
  buzzer.clear();
}
