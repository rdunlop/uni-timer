#include "uni_gps.h"
#include "uni_sd.h"
#include "uni_buzzer.h"
#include "uni_sensor.h"
#include "modes.h"
#include "recording.h"
#include "accurate_timing.h"
#include "event_queue.h"

extern UniGps gps;
//extern UniPrinter printer;
extern UniSd sd;
extern UniSensor sensor;
extern UniBuzzer buzzer;

boolean countdown_finished = true;

/*********************************************************************************** */
//### Mode 5 - Race Run (Start Line, beep start)
//

// *****************************************************

// This is the FSM action which occurs after
// we notice that the sensor interrupt has fired.
void mode4_sensor_triggered(char *event_data) {
  Serial.println("SENSOR TRIGGERED");
  char result[EVT_MAX_STR_LEN];
  snprintf(result, EVT_MAX_STR_LEN, "%s,FS", event_data);
  if (racer_number()) {
    buzzer.warning();
    publish_time_recorded(racer_number(), result);
    clear_racer_number();
  } else {
    buzzer.success();
  }
}

void mode4_setup() {
  countdown_finished = true;
  clear_racer_number();
}

void mode4_event_handler(uint8_t event_type, char *event_data) {
//  Serial.println("Mode 4 event handler");
  switch(event_type) {
    case EVT_DELETE_RESULT:
      buzzer.error(); // TBD
      break;
    case EVT_SENSOR_BLOCKED:
      // A racer crossed the line, and we have recorded the time and the (possible) penalty
      mode4_sensor_triggered(event_data);
      break;
    case EVT_RACER_NUMBER_ENTERED:
      countdown_finished = false;
      buzzer.countdown();
      store_racer_number(atoi(event_data));
      break;
    case EVT_TIMER_COUNTDOWN_FINISHED:
      countdown_finished = true;
      if (racer_number()) {
        publish_time_recorded(racer_number(), event_data);
        clear_racer_number();
      }
      break;
  }
}

void mode4_teardown() {
}
