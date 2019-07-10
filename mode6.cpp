#include "uni_keypad.h"
#include "uni_gps.h"
#include "uni_display.h"
#include "uni_printer.h"
#include "uni_sd.h"
#include "uni_buzzer.h"
#include "uni_sensor.h"
#include "modes.h"
#include "recording.h"

extern UniKeypad keypad;
extern UniGps gps;
extern UniDisplay display;
extern UniPrinter printer;
extern UniSd sd;
extern UniSensor sensor;
extern UniBuzzer buzzer;

#include <Fsm.h>

// ***************************************************** MODE 6 ***************************************
//### Mode 6 - Race Run (Finish Line)
//
//- When a sensor is triggered, display E1 to indicate that you need to enter 1 racer number.
//  - It will beep periodically to indicate this
//  - If you have 2 times recorded, it will beep twice periodically, etc.
//- when you press number keys, display the numbers on the display.
//- If you enter more than 3 digits, it will beep and clear
//- If you press "A", it will accept the input, and display the time and the racer number to printer/SD
//- If you press "D", it will clear the display
//- If you press "B", it will duplicate the last time, and create E2 (only available from initial mode)
//- If you press C+* it will clear the last entry

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

Fsm mode6_fsm(&mode6_initial);
#define MAX_RESULTS 10
char results_to_record[MAX_RESULTS][20];
int results_count = 0;

void store_data_result(char *data) {
  if (results_count < MAX_RESULTS) {
    sprintf(results_to_record[results_count], data);
    results_count ++;
    Serial.println("stored new result");
  } else {
    Serial.println("Results cache is full");
  }
}

// Are there any results in the buffer, if so,
// return the oldest one
bool retrieve_data_string(char *str) {
  // TODO: Pause interrupts during this function?
  if (results_count > 0) {
    strcpy(str, results_to_record[0]);

    // Copy the remaining results up 1 slot
    for (int i = 0; i < (results_count - 1); i++) {
      strcpy(results_to_record[i], results_to_record[i + 1]);
    }
    results_count--;
    
    return true;
  }
  return false;
}

void store_timing_data() {
  Serial.println("SENSOR TRIGGERED");
  Serial.println(sensor.interrupt_micros());
  
  buzzer.beep();
//  display.sens();
  char data_string[25];
  currentTime(sensor.interrupt_micros(), data_string);
  store_data_result(data_string);
  Serial.println(data_string);
  
  sensor.clear_interrupt_micros();
  delay(500);
  // DELAY 500 ms before able to read interrupt again
}

// While waiting for a new datapoint
// Watch for sensor, etc
void mode6_initial_check() {
  if (sensor.blocked_via_interrupt()) {
    mode6_fsm.trigger(SENSOR);
  }
  
  char last_key_pressed = keypad.readChar();
  if (keypad.isDigit(last_key_pressed)) {
    mode6_fsm.trigger(NUMBER_PRESSED);
  } else if (keypad.keyPressed('C') && keypad.keyPressed('*')) { // C+*
    // TODO: SHOULD CLEAR Previous Racer's time
    Serial.println("TO CLEAR");
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
  sensor.attach_interrupt(); 
}

void mode6_teardown() {
  sensor.detach_interrupt();
}

// When a digit has been entered, monitor for A, D, #
void mode6_digit_check() {
  if (sensor.blocked_via_interrupt()) {
    mode6_fsm.trigger(SENSOR);
  }
  
  char last_key_pressed = keypad.readChar();
  if (keypad.isDigit(last_key_pressed)) {
    if (three_digits_racer_number()) {
      mode6_fsm.trigger(DELETE);
    } else {
      mode6_fsm.trigger(NUMBER_PRESSED);
    }
  } else if (keypad.keyPressed('D')) {
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
  char full_string[25];
  char data_string[25];
  if (retrieve_data_string(data_string)) {
    // There is data to be stored
    char filename[20];
    build_race_filename(filename);
    sprintf(full_string, "RACER %d - %s", racer_number(), data_string);
    Serial.println(full_string);
    printer.print(full_string);
    sd.writeFile(filename, full_string);
    clear_racer_number();  
  }
}
