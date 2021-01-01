#include "uni_buzzer.h"
#include "uni_sensor.h"
#include "modes.h"
#include "event_queue.h"

extern UniSensor sensor;
extern UniBuzzer buzzer;

//### Mode 1 - Sensor Input Test
//
//- If you cross the sensor, beep for 100ms
void mode1_event_handler(uint8_t event_type, char *event_data) {
  switch(event_type) {
    case EVT_SENSOR_CHANGE:
      buzzer.beep(1000);
      break;
  }
}
