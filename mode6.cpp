#include "uni_gps.h"
#include "uni_sd.h"
#include "uni_buzzer.h"
#include "uni_sensor.h"
#include "modes.h"
#include "recording.h"
#include "accurate_timing.h"
#include "event_queue.h"

extern UniGps gps;
extern UniSd sd;
extern UniSensor sensor;
extern UniBuzzer buzzer;

// ***************************************************** MODE 6 ***************************************
//### Mode 6 - Race Run (Finish Line)
//
//- When a sensor is triggered (EVT_SENSOR_CHANGE), it will publish a EVT_TIME_RECORD event ##,hh:mm:ss.zzzz/###
//- When a racer number is entered (EVT_RACER_NUMBER_ENTERED), it will store the recorded time to the SD card and publish a EVT_TIME_STORED event
//- When a EVT_DUPLICATE is received it will create a new time entry, and publish EVT_TIME_RECORD
//- When a EVT_DELETE is received, it will delete the current time entry

#define MAX_RESULTS 10
char results_to_record[MAX_RESULTS][EVT_MAX_STR_LEN];
int results_count = 0;

void store_data_result(char *data) {
  if (results_count < MAX_RESULTS) {
    strcpy(results_to_record[results_count], data);
    results_count ++;
    Serial.println("stored new result");
  } else {
    Serial.println("Results cache is full");
  }
}

// Are there any results in the buffer, if so,
// return the oldest one
bool retrieve_data(char *data) {
  // TODO: Pause interrupts during this function?
  if (results_count > 0) {
    strcpy(data, results_to_record[0]);

    // Copy the remaining results up 1 slot
    for (int i = 0; i < (results_count - 1); i++) {
      strcpy(results_to_record[i], results_to_record[i + 1]);
    }
    results_count--;

    return true;
  }
  return false;
}

// Create a second entry of the most recently-recorded data
void duplicate_entry() {
  if (results_count > 0 && (results_count < MAX_RESULTS)) {
    strcpy(results_to_record[results_count - 1], results_to_record[results_count]);
    results_count += 1;
    buzzer.beep();
  }
}

// If we want to remove an entry, for example: incorrectly counted 2 crossings.
void drop_last_entry() {
  if (results_count > 0) {
    results_count -= 1;
  }
}

void store_timing_data(char *event_data) {
  Serial.println("SENSOR TRIGGERED");

  buzzer.beep();
  store_data_result(event_data);
}

void mode6_event_handler(uint8_t event_type, char *event_data) {
  Serial.println("Mode 6 event handler");
  switch(event_type) {
    case EVT_DUPLICATE_RESULT:
      duplicate_entry();
      break;
    case EVT_DELETE_RESULT:
      drop_last_entry();
      break;
    case EVT_SENSOR_BLOCKED:
      store_timing_data(event_data);
      break;
    case EVT_RACER_NUMBER_ENTERED:
      store_racer_number(atoi(event_data));
      char result_data[EVT_MAX_STR_LEN];
      if (retrieve_data(result_data)) {
        push_racer_number(racer_number(), result_data);
        clear_racer_number();
      }
      clear_racer_number();
    break;
  }
}


void mode6_setup() {
  results_count = 0;
}

void mode6_teardown() {
}
