#include "uni_keypad.h"
#include "uni_gps.h"
#include "uni_display.h"
#include "uni_buzzer.h"
#include "uni_sensor.h"
#include "uni_radio.h"
#include "modes.h"
#include "recording.h"
#include "accurate_timing.h"

extern UniKeypad keypad;
extern UniGps gps;
extern UniDisplay display;
extern UniSensor sensor;
extern UniBuzzer buzzer;
extern UniRadio radio;

#include <Fsm.h>

// ***************************************************** MODE 6 ***************************************
//### Mode 6 - Race Run (Finish Line)
//
//- When a sensor is triggered, display E1 to indicate that you need to enter 1 racer number.
//  - It will beep periodically to indicate this
//  - If you have 2 times recorded, it will beep twice periodically, etc.
//- when you press number keys, display the numbers on the display.
//- If you enter more than 3 digits, it will beep and clear
//- If you press "A", it will accept the input, and display the time and the racer number to SD
//- If you press "C", it will clear the display
//- If you press "B", it will duplicate the last time received, and create E2 (only available when no racer number is entered)
//- If you press D+* it will clear the last entry

#define NUMBER_PRESSED 1
#define DELETE 2
#define ACCEPT 3
#define CANCEL 4
#define SENSOR 5

void mode6_initial_entry();
void mode6_initial_check();
void mode6_initial_exit();
void mode6_digit_check();
void mode6_store_result();

State mode6_initial(&mode6_initial_entry, &mode6_initial_check, &mode6_initial_exit);
State mode6_digits_entered(NULL, &mode6_digit_check, NULL);
bool fsm_6_transition_setup_complete = false;

Fsm mode6_fsm(&mode6_initial);
#define MAX_RESULTS 20
// Oldest result is at index 0
// Newest result is at index [results_count - 1]
TimeResult results_to_record[MAX_RESULTS];
int results_count = 0;

void store_data_result(TimeResult *data) {
  if (results_count < MAX_RESULTS) {
    results_to_record[results_count] = *data;
    results_count ++;
    Serial.println("stored new result");
  } else {
    Serial.println("Results cache is full");
  }
}

// Are there any results in the buffer, if so,
// return the oldest one
bool retrieve_data(TimeResult *data) {
  // TODO: Pause interrupts during this function?
  if (results_count > 0) {
    *data = results_to_record[0];

    // Copy the remaining results up 1 slot
    for (int i = 0; i < (results_count - 1); i++) {
      results_to_record[i] = results_to_record[i + 1];
    }
    results_count--;
    
    return true;
  }
  return false;
}

// Create a second entry of the most recently-received data
void duplicate_entry() {
  if (results_count > 0 && (results_count < MAX_RESULTS)) {
    log("Duplicate entry");
    results_to_record[results_count] = results_to_record[results_count - 1];
    results_count += 1;
    buzzer.beep();
    display.showEntriesRemaining(results_count);
  }
}

// If we want to remove an entry, for example: incorrectly counted 2 crossings.
void drop_last_entry() {
  log("drop last entry");
  if (results_count > 0) {
    results_count -= 1;
    display.showEntriesRemaining(results_count);
  }
}

void store_timing_data() {
  Serial.println("SENSOR TRIGGERED");
  
  buzzer.beep();
  TimeResult data;
  lastSensorTime(&data);
  store_data_result(&data);

  clear_sensor_interrupt_millis();
}

bool deleting = false;
// While waiting for a new datapoint
// Watch for sensor, etc
void mode6_initial_check() {
  if (sensor_has_triggered()) {
    mode6_fsm.trigger(SENSOR);
  }
  
  char last_key_pressed = keypad.readChar();
  if (keypad.isDigit(last_key_pressed)) {
    mode6_fsm.trigger(NUMBER_PRESSED);
  } else if (last_key_pressed == 'B') {
    duplicate_entry();
  } else if (!deleting && keypad.keyPressed('D') && keypad.keyPressed('#')) { // D+#
    drop_last_entry();
    deleting = true;
  } else if (deleting && !keypad.anyKeyPressed()) {
    deleting = false;
  }
#ifdef FSM_DEBUG
  Serial.println("Initial Check ");
#endif
}

void mode6_initial_entry() {
  display.showEntriesRemaining(results_count);
}

void mode6_initial_exit() {
}

void mode6_loop() {
  mode6_fsm.run_machine();
}

void mode6_fsm_setup() {
  mode6_fsm.add_transition(&mode6_initial, &mode6_digits_entered, NUMBER_PRESSED, &store_racer_number);
  mode6_fsm.add_transition(&mode6_initial, &mode6_initial, SENSOR, &store_timing_data);
  
  mode6_fsm.add_transition(&mode6_digits_entered, &mode6_initial, DELETE, &clear_racer_number);
  mode6_fsm.add_transition(&mode6_digits_entered, &mode6_digits_entered, NUMBER_PRESSED, &store_racer_number);
  mode6_fsm.add_transition(&mode6_digits_entered, &mode6_initial, ACCEPT, &mode6_store_result);
  mode6_fsm.add_transition(&mode6_digits_entered, &mode6_digits_entered, SENSOR, &store_timing_data);
}

void mode6_setup() { 
  if (!fsm_6_transition_setup_complete)  {
    Serial.println("Mode6 Setup complete");
    mode6_fsm_setup();
    fsm_6_transition_setup_complete = true;
  }
  Serial.println("starting mode 6");
  display.clear();
  sensor.attach_interrupt(); 
}

void mode6_teardown() {
  sensor.detach_interrupt();
}

// When a digit has been entered, monitor for A, C, #
void mode6_digit_check() {
  if (sensor_has_triggered()) {
    mode6_fsm.trigger(SENSOR);
  }
  
  char last_key_pressed = keypad.readChar();
  if (keypad.isDigit(last_key_pressed)) {
    if (maximum_digits_racer_number()) {
      mode6_fsm.trigger(DELETE);
    } else {
      mode6_fsm.trigger(NUMBER_PRESSED);
    }
  } else if (keypad.keyPressed('C')) {
    mode6_fsm.trigger(DELETE);
  } else if (keypad.keyPressed('A')) {
    mode6_fsm.trigger(ACCEPT);
  }
#ifdef FSM_DEBUG
  Serial.println("Digit Check ");
#endif
}

// Store the racer number and time together in a file
void mode6_store_result() {
  Serial.println("STORE RESULT");
  
  buzzer.beep();
  TimeResult data;
  if (retrieve_data(&data)) {
    if (print_racer_data_to_sd(racer_number(), data)) {
      char message[25];
      format_string(racer_number(), data, false, message, 25);
      char full_message[27];
      snprintf(full_message, 27, "%s,%s", "F", message);
      radio.queueToSend(full_message);

      clear_racer_number();
    } else {
      display.sdBad();
      buzzer.failure();
    }
  }
}
