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

/*********************************************************************************** */
//### Mode 5 - Race Run (Start Line)
//
// - After entering a Racer Number, the system will start as soon as the user crosses the start line. (EVT_RACER_NUMBER_ENTERED)
//   - Once entered, success music is played
//   - Every 1 seconds it will do a short-beep every second until the racer starts
// - You can also clear the racer number (EVT_RACER_NUMBER_ENTERED with "")
// - When the racer crosses the line, we record their time, and beep success.
// - If no racer nuber is entered, and the sensor is crossed, we error-beep
// - If you need to cancel the previous rider's start time, EVT_DELETE_RESULT
//

// *****************************************************

// This is the FSM action which occurs after
// we notice that the sensor interrupt has fired.
void sensor_triggered(char *event_data) {
  if (racer_number()) {
    buzzer.success();
    push_racer_number(racer_number(), event_data);
    clear_racer_number();
  } else {
    buzzer.error();
  }
}

void mode5_setup() {
  clear_racer_number();
}

void mode5_event_handler(uint8_t event_type, char *event_data) {
  Serial.println("Mode 5 event handler");
  switch(event_type) {
    case EVT_TIME_CHANGE:
      if (racer_number()) {
        buzzer.beep(300);
      }
      break;
    case EVT_DELETE_RESULT:
      buzzer.error(); // TBD
      break;
    case EVT_SENSOR_BLOCKED:
      sensor_triggered(event_data);
      break;
    case EVT_RACER_NUMBER_ENTERED:
      if (racer_number()) {
        buzzer.error();
      } else {
        store_racer_number(atoi(event_data));
      }
      break;
  }
}

void mode5_teardown() {
}
