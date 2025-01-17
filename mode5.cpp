#include "uni_keypad.h"
#include "uni_gps.h"
#include "uni_display.h"
#include "uni_buzzer.h"
#include "uni_sensor.h"
#include "uni_config.h"
#include "modes.h"
#include "recording.h"
#include "accurate_timing.h"

extern UniKeypad keypad;
extern UniGps gps;
extern UniDisplay display;
extern UniSensor sensor;
extern UniBuzzer buzzer;
extern UniConfig config;

#include <Fsm.h>

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

void initial_check() {
  char last_key_pressed = keypad.readChar();
  if (keypad.isDigit(last_key_pressed)) {
    mode5_fsm.trigger(NUMBER_PRESSED);
  } else if(sensor.blocked()) {
    buzzer.beep();
    display.sens();
  } else if (keypad.keyPressed('D') && keypad.keyPressed('#')) { // D+#
    log("Clear previous entry");
    clear_previous_entry();
  }
#ifdef FSM_DEBUG
  Serial.println("Initial Check ");
#endif
}

void digit_check() {
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
    mode5_fsm.trigger(DELETE);
    log("CLEARED RACER NUMBER");
  } else if (sensor.blocked()) {
    TimeResult data;
    currentTime(&data);
    print_data_to_log(data);
    buzzer.beep();
    display.sens();
  }
#ifdef FSM_DEBUG
  Serial.println("Digit Check");
#endif
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
#ifdef FSM_DEBUG
  Serial.println("Sensor Check");
#endif
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
}

// the final BEEP has triggered, and the sensor has not been crossed
// THUS we store the current time, no fault
void start_beeped() {
  Serial.println("START BEEPED");
  Serial.println(millis());
  TimeResult data;
  currentTime(&data);

  if (print_racer_data_to_sd(racer_number(), data)) {
    buzzer.start_beep();
  } else {
    display.sd();
    buzzer.failure();
  }

  clear_racer_number();
  clear_sensor_interrupt_millis();
}

// This is the FSM action which occurs after
// we notice that the sensor interrupt has fired.
void sensor_triggered() {
  Serial.println("SENSOR TRIGGERED 5");
  Serial.println(sensor_interrupt_millis());
  
  display.sens();
  
  TimeResult data;
  lastSensorTime(&data);
  // QUESTION: If someone faults, what time should be recorded? Their ACTUAL start time + penalty, right?
  if (config.get_start_line_countdown()) {
    if (print_racer_data_to_sd(racer_number(), data, true)) {
      // fault, but let them race
      buzzer.failure();
    } else {
      display.sd();
    }
  } else {
    if (print_racer_data_to_sd(racer_number(), data)) {
      buzzer.beep();
    } else {
      buzzer.failure();
      display.sd();
      delay(2000);
    }
  }
  print_data_to_log(data);
  
  clear_racer_number();
  clear_sensor_interrupt_millis();
}

void sensor_entry() {
  log("ACCEPTED");
  clear_sensor_interrupt_millis();
  display.setBlink(true);
}

void sensor_exit() {
  Serial.println("exiting");
  display.setBlink(false);
  countdown_start_time = 0;
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

void mode5_fsm_setup() {
  mode5_fsm.add_transition(&initial, &digits_entered, NUMBER_PRESSED, &store_racer_number);
  
  mode5_fsm.add_transition(&digits_entered, &initial, DELETE, &clear_racer_number);
  mode5_fsm.add_transition(&digits_entered, &digits_entered, NUMBER_PRESSED, &store_racer_number);
  mode5_fsm.add_transition(&digits_entered, &ready_for_sensor, ACCEPT, NULL);

  mode5_fsm.add_transition(&ready_for_sensor, &initial, SENSOR, &sensor_triggered);
  mode5_fsm.add_transition(&ready_for_sensor, &initial, START, &start_beeped);
  mode5_fsm.add_transition(&ready_for_sensor, &initial, DELETE, &clear_racer_number);
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
