#include "uni_buzzer.h"
#include "modes.h"
#include "recording.h"
#include "accurate_timing.h"
#include "event_queue.h"

//extern UniPrinter printer;
extern UniBuzzer buzzer;

/*********************************************************************************** */
//### Mode 5 - Race Run (Start Line, beep start)
//

// *****************************************************

// This is the FSM action which occurs after
// we notice that the sensor interrupt has fired.
void mode4_sensor_triggered(char *event_data) {
  Serial.println("Sensor triggered before countdown completed");
  if (racer_number()) {
    buzzer.warning();
    Serial.println("Logging False-start for racer");
    char result[EVT_MAX_STR_LEN];
    snprintf(result, EVT_MAX_STR_LEN, "%s,FS", event_data);
    push_racer_number(racer_number(), result);
    clear_racer_number();
  } else {
    Serial.println("no racer was entered yet");
    buzzer.success();
  }
}

void mode4_setup() {
  clear_racer_number();
  buzzer.clear();
}

// callback function to be called when the countdown
// finishes
void cb() {
  char result[EVT_MAX_STR_LEN];
  TimeResult output;
  currentTime(&output);
  format_time_result(&output, result, EVT_MAX_STR_LEN);
  push_event(EVT_TIMER_COUNTDOWN_FINISHED, result);
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
      buzzer.countdown(cb);
      store_racer_number(atoi(event_data));
      break;
    case EVT_TIMER_COUNTDOWN_FINISHED:
      if (racer_number()) {
        push_racer_number(racer_number(), event_data);
        clear_racer_number();
      }
      break;
  }
}

void mode4_teardown() {
  buzzer.clear();
  clear_racer_number();
}
