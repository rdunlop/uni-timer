#include "uni_keypad.h"
#include "uni_gps.h"
#include "uni_display.h"
#include "uni_buzzer.h"
#include "uni_sensor.h"
#include "uni_config.h"
#include "uni_radio.h"
#include "modes.h"
#include "recording.h"
#include "accurate_timing.h"

extern UniKeypad keypad;
extern UniGps gps;
extern UniDisplay display;
extern UniSensor sensor;
extern UniBuzzer buzzer;
extern UniConfig config;
extern UniRadio radio;

#include <Fsm.h>

// #define SIMULATE true
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
//    - write the current time to the SD
//    - display 5En5 on the display for 2 seconds and beep for 2 seconds.
//- When NOT in Accepted State:
//  - If the sensor is crossed
//    - display Err and beep
//- Press C+* If you need to cancel the previous rider's start time.
//  - This will print and record the cancellation of the previous start time
//

// *****************************************************
// Mode 5 FSM

// Data

void good_music() {
  buzzer.success();
}

void mode5_store_racer_number();
void mode5_clear_racer_number();

void initial_check();
void digit_check();
void sensor_check();
void sensor_entry();
void sensor_exit();

State initial(NULL, &initial_check, NULL);
State digits_entered(NULL, &digit_check, NULL);
State ready_for_sensor(&sensor_entry, &sensor_check, &sensor_exit);
bool fsm_5_transition_setup_complete = false;

Fsm mode5_fsm(&initial);

#define NUMBER_PRESSED 1
#define DELETE 2
#define ACCEPT 3
#define CANCEL 4
#define SENSOR 5
#define START 6

void mode5_store_timing_data() {
  // Log any crossings to the file
  buzzer.beep();
  display.sens();
  TimeResult data;
  lastSensorTime(&data);
  print_data_to_log(data);
  clear_sensor_interrupt_millis();
}
void initial_check() {
  if (sensor_has_triggered()) {
    mode5_fsm.trigger(SENSOR);
  }
  char last_key_pressed = keypad.readChar();
  if (keypad.isDigit(last_key_pressed)) {
    mode5_fsm.trigger(NUMBER_PRESSED);
  } else if (sensor.blocked()) {
    buzzer.beep();
  } else if (keypad.keyPressed('D') && keypad.keyPressed('#')) { // D+#
    log("Clear previous entry");
    clear_previous_entry();
  }
}

void digit_check() {
  if (sensor_has_triggered()) {
    mode5_fsm.trigger(SENSOR);
  }
  // - 0-9 -> TWO_DIGITS_ENTERED or THREE_DIGITS_ENTERED
  // - A -> ACCEPTING
  // - C -> INITIAL
  char last_key_pressed = keypad.readChar();
  if (keypad.isDigit(last_key_pressed)) {
    if (maximum_digits_racer_number()) {
      mode5_fsm.trigger(DELETE);
    } else {
      mode5_fsm.trigger(NUMBER_PRESSED);
    }
  } else if (last_key_pressed == 'A') {
    mode5_fsm.trigger(ACCEPT);
  } else if (last_key_pressed == 'C') {
    display.clear();
    mode5_fsm.trigger(DELETE);
    log("CLEARED RACER NUMBER");
  } else if (sensor.blocked()) {
    buzzer.beep();
  }
}

void countdown(); // forward declaration

void sensor_check() {
  if (keypad.newKeyPressed() && keypad.keyPressed('C')) {
    mode5_fsm.trigger(DELETE);
    log("DELETED RACER NUMBER");
  } else if (sensor_has_triggered()) {
    mode5_fsm.trigger(SENSOR);
  } else if (config.get_start_line_countdown()) {
    // In Countdown mode
    countdown();
  }
}

uint32_t countdown_start_time = 0;
uint8_t countdown_step = 0;

// Perform countdown beep, beep, beep, beep, beep, BEEP
// If someone crosses the line before the BEEP starts
// triggers a FAULT event (after the SENSOR event caused by the user)
// Otherwise, triggers a FINAL_BEEP event (before the SENSOR event caused by the user)
void countdown() {
  if (countdown_start_time == 0) {
    // start the countdown
    Serial.println("Restarting countdown");
    countdown_start_time = millis();
    buzzer.pre_beep(); // beep for 0.5 second for each tone
    countdown_step = 1;
  }
  switch (config.get_start_line_countdown_mode()) {
  case 0:
    // there are 4 additional pre-beeps (5 total pre-beeps)

    // Second tone occurs at 3 seconds
    if (countdown_step < 5 && countdown_start_time + (1000 * countdown_step) < millis()) {
      buzzer.pre_beep();
      countdown_step += 1;
    }

    // last (ie: START) tone occurs at 7 seconds
    if (countdown_step == 5 && countdown_start_time + (1000 * countdown_step) < millis()) {
      mode5_fsm.trigger(START);
      countdown_step += 1;
    }
    break;
  case 1:
    if (countdown_step == 1 && (countdown_start_time + 3000) <= millis()) {
      // The rider didn't cross the line before the countdown expired
      mode5_fsm.trigger(START);
      countdown_step += 1;
    }
    break;
  }
}

// the final BEEP has triggered, and the sensor has not been crossed
// THUS we store the current time, no fault
void start_beeped() {
  #ifndef SIMULATE
  buzzer.start_beep();
  #endif

  Serial.println("START BEEPED");
  Serial.println(millis());
  TimeResult data;
  currentTime(&data);

  if (print_racer_data_to_sd(racer_number(), data)) {
    // do the beep earlier, because we don't want to incur
    // the sd-card-write delay
  } else {
    display.sdBad();
    buzzer.failure();
    delay(2000);
  }
  char message[25];
  format_string(racer_number(), data, false, message, 25);
  char full_message[27];
  snprintf(full_message, 27, "%s,%s", "S", message);
  radio.queueToSend(full_message);

  mode5_clear_racer_number();
  clear_sensor_interrupt_millis();
}

// This is the FSM action which occurs after
// we notice that the sensor interrupt has fired.
void sensor_triggered() {
  Serial.println("SENSOR TRIGGERED 5");
  
  display.sens();
  
  TimeResult data;
  lastSensorTime(&data);
  // QUESTION: If someone faults, what time should be recorded? Their ACTUAL start time + penalty, right?
  if (config.get_start_line_countdown()) {
    bool fault;
    if (config.get_start_line_countdown_mode() == 0) {
      fault = true;
    } else {
      // in start_line_countdown_mode == 1, it's not a fault
      // to cross the line before the final buzzer.
      fault = false;
    }
    // fault, but let them race
    if (fault) {
      buzzer.fault();
    } else {
      buzzer.start_beep();
    }
    if (print_racer_data_to_sd(racer_number(), data, fault)) {
      // Perform the buzzer earlier, because we don't want to
      // afford the sd-write delay
    } else {
      display.sdBad();
      delay(2000);
    }
    print_data_to_log(data, true);

    char message[25];
    format_string(racer_number(), data, true, message, 25);
    char full_message[27];
    snprintf(full_message, 27, "%s,%s", "S", message);
    radio.queueToSend(full_message);
  } else {
    if (print_racer_data_to_sd(racer_number(), data)) {
      buzzer.beep();
    } else {
      buzzer.failure();
      display.sdBad();
      delay(2000);
    }
    print_data_to_log(data);
    char message[25];
    format_string(racer_number(), data, false, message, 25);
    char full_message[27];
    snprintf(full_message, 27, "%s,%s", "S", message);
    radio.queueToSend(full_message);
  }
  
  mode5_clear_racer_number();
  clear_sensor_interrupt_millis();
}

void sensor_entry() {
  clear_sensor_interrupt_millis();
  #ifndef SIMULATE
  display.waitingForSensor(racer_number(), config.get_start_line_countdown());
  #endif
}

void sensor_exit() {
  display.doneWaitingForSensor();
  countdown_start_time = 0;
}

void mode5_store_racer_number() {
  display.showNumber(store_racer_number());
}

void mode5_clear_racer_number() {
  clear_racer_number();
  display.clear();
}

/*
 * Possible Actions:
 * Sensor
 * Number
 * C
 * A
 * D*
 * 
 * Possible States:
 * INITIAL
 * DIGITS
 * READY
 */

#ifdef SIMULATE
extern int _racer_number;
int simulated_racer_number = 1;
void simulate_racer_number() {
  _racer_number = simulated_racer_number;
  char str1[20], str2[20];
  snprintf(str1, 20, "Uptime: %ld min", millis() / 1000 / 60);
  snprintf(str2, 20, "Racer: %d", _racer_number);
  display.print(str1, str2);
  #ifdef SEVEN_SEGMENT_DISPLAY
  display.showNumber(millis() / 1000 / 60);
  #endif
  simulated_racer_number += 1;
}

#endif
void mode5_fsm_setup() {
  #ifdef SIMULATE
  mode5_fsm.add_timed_transition(&initial, &ready_for_sensor, 300, &simulate_racer_number);
  mode5_fsm.add_timed_transition(&ready_for_sensor, &initial, 500, &start_beeped);
  #endif
  mode5_fsm.add_transition(&initial, &digits_entered, NUMBER_PRESSED, &mode5_store_racer_number);
  mode5_fsm.add_transition(&initial, &initial, SENSOR, &mode5_store_timing_data);
  
  mode5_fsm.add_transition(&digits_entered, &initial, DELETE, &mode5_clear_racer_number);
  mode5_fsm.add_transition(&digits_entered, &digits_entered, NUMBER_PRESSED, &mode5_store_racer_number);
  mode5_fsm.add_transition(&digits_entered, &ready_for_sensor, ACCEPT, NULL);
  mode5_fsm.add_transition(&digits_entered, &digits_entered, SENSOR, &mode5_store_timing_data);

  mode5_fsm.add_transition(&ready_for_sensor, &initial, SENSOR, &sensor_triggered);
  mode5_fsm.add_transition(&ready_for_sensor, &initial, START, &start_beeped);
  mode5_fsm.add_transition(&ready_for_sensor, &initial, DELETE, &mode5_clear_racer_number);
}

void mode5_setup() {
  if (!fsm_5_transition_setup_complete) {
    mode5_fsm_setup();
    fsm_5_transition_setup_complete = true;
  }
  Serial.println("starting mode 5");
  display.clear();
  sensor.attach_interrupt();
  // States:
  // INITIAL
  // DIGITS_ENTERED
  // READY_FOR_SENSOR

  // Transitions
  // INITIAL:
  // - 0-9 -> DIGITS_ENTERED
  // - C   -> (Cancel Previous Data AND) INITIAL
  // - D+* -> Delete last entry
  // - SENSOR -> Beep
  // DIGITS_ENTERED:
  // - 0-9 -> DIGITS_ENTERED
  // - A -> ACCEPTING
  // - D -> INITIAL
  // READY_FOR_SENSOR:
  // - SENSOR -> RECORD
  // - D -> Clear
  

  // Entry/Exit
  // INITIAL:
  // - Clear the racer number
  // DIGITS_ENTERED
  // - On Transition -> Store current keypress
  // READY_FOR_SENSOR
  // - On Entry -> Success Music
}
void mode5_loop() {
  mode5_fsm.run_machine();
}

void mode5_teardown() {
  sensor.detach_interrupt();
}
