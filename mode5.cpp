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
//- If you enter a number on the keypad, display that number, and allow up to 3 numbers to be entered.
//- If you enter a 4th number, beep and clear the display.
//- If you press A, it "Accepts" the number, and makes success music, and continues to show the number on the display.
//- Once Accepted, blink the number on the display every second
//- If you press D, it clears the number and leaves "Accepted" mode
//- When in Accepted state:
//  - If the sensor is crossed
//    - write the current time to the SD and the printer
//    - display 5En5 on the display for 2 seconds and beep for 2 seconds.
//- When NOT in Accepted State:
//  - If the sensor is crossed
//    - display Err and beep
//- Press C+* If you need to cancel the previous rider's start time.
//  - This will print and record the cancellation of the previous start time
//

// *****************************************************

// This is the FSM action which occurs after
// we notice that the sensor interrupt has fired.
void sensor_triggered(char *event_data) {
  Serial.println("SENSOR TRIGGERED");

  if (racer_number()) {
    buzzer.success();
    push_racer_number(racer_number(), event_data);
  } else {
    buzzer.error();
  }

  clear_racer_number();
}

void mode5_setup() {
  clear_racer_number();
}

void mode5_event_handler(uint8_t event_type, char *event_data) {
  Serial.println("Mode 5 event handler");
  switch(event_type) {
    case EVT_DELETE_RESULT:
      buzzer.error(); // TBD
      break;
    case EVT_SENSOR_BLOCKED:
      sensor_triggered(event_data);
      break;
    case EVT_RACER_NUMBER_ENTERED:
      store_racer_number(atoi(event_data));
      break;
  }
}

void mode5_teardown() {
}
