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
State one_digit_entered(NULL, &digit_check, NULL);
State two_digits_entered(NULL, &digit_check, NULL);
State three_digits_entered(NULL, &digit_check, NULL);
State ready_for_sensor(&sensor_entry, &sensor_check, &sensor_exit);

Fsm mode5_fsm(&initial);

unsigned long _sensor_micros = 0;

#define NUMBER_PRESSED 1
#define DELETE 2
#define ACCEPT 3
#define CANCEL 4
#define SENSOR 5

void initial_check() {
  char last_key_pressed = keypad.readChar();
  if (keypad.isDigit(last_key_pressed)) {
    mode5_fsm.trigger(NUMBER_PRESSED);
  } else if(sensor.blocked()) {
    buzzer.beep();
    display.sens();
    delay(100);
  } else if (keypad.keyPressed('C') && keypad.keyPressed('*')) { // C+*
    // TODO: SHOULD CLEAR Previous Racer's time
    Serial.println("TO CLEAR");
  }
#ifdef FSM_DEBUG
  Serial.println("Initial Check ");
#endif
}

void digit_check() {
  // - 0-9 -> TWO_DIGITS_ENTERED or THREE_DIGITS_ENTERED
  // - A -> ACCEPTING
  // - D -> INITIAL
  char filename[20];
  build_race_filename(filename);
  char last_key_pressed = keypad.readChar();
  if (keypad.isDigit(last_key_pressed)) {
    mode5_fsm.trigger(NUMBER_PRESSED);
  } else if (last_key_pressed == 'A') {
    mode5_fsm.trigger(ACCEPT);
  } else if (last_key_pressed == 'B') {
    sd.readFile(filename);
  } else if (last_key_pressed == 'D') {
    mode5_fsm.trigger(DELETE);
  } else if (sensor.blocked()) {
    buzzer.beep();
    display.sens();
  }
#ifdef FSM_DEBUG
  Serial.println("Digit Check");
#endif
}

void sensor_check() {
  if (keypad.newKeyPressed() && keypad.keyPressed('D')) {
    Serial.println("D PRessed");
    mode5_fsm.trigger(DELETE);
  } else if (sensor.blocked_via_interrupt()) {
    mode5_fsm.trigger(SENSOR);
  }
#ifdef FSM_DEBUG
  Serial.println("Sensor Check");
#endif
}

// This is the FSM action which occurs after
// we notice that the sensor interrupt has fired.
void sensor_triggered() {
  Serial.println("SENSOR TRIGGERED");
  Serial.println(sensor.interrupt_micros());
  
  buzzer.beep();
  display.sens();
  char full_string[25];
  char data_string[25];
  char filename[20];
  build_race_filename(filename);
  currentTime(sensor.interrupt_micros(), data_string);
  sprintf(full_string, "RACER %d - %s", racer_number(), data_string);
  Serial.println(full_string);
  printer.print(full_string);
  sd.writeFile(filename, full_string);
  clear_racer_number();
  sensor.clear_interrupt_micros();
}

void sensor_entry() {
  _sensor_micros = 0;
  display.setBlink(true);
}

void sensor_exit() {
  display.setBlink(false);
}

void sensor_interrupt() {
  _sensor_micros = micros();
  Serial.println("INTERRUPTED");
  Serial.println(_sensor_micros);
}


/*
 * Possible Actions:
 * Sensor
 * Number
 * C*
 * A
 * D
 * 
 * Possible States:
 * INITIAL
 * ONE
 * TWO
 * THREE
 * READY
 */

void mode5_fsm_setup() {
   mode5_fsm.add_transition(&initial, &one_digit_entered, NUMBER_PRESSED, &store_racer_number);
  
  mode5_fsm.add_transition(&one_digit_entered, &initial, DELETE, &clear_racer_number);
  mode5_fsm.add_transition(&one_digit_entered, &two_digits_entered, NUMBER_PRESSED, &store_racer_number);
  mode5_fsm.add_transition(&one_digit_entered, &ready_for_sensor, ACCEPT, NULL);

  mode5_fsm.add_transition(&two_digits_entered, &initial, DELETE, &clear_racer_number);
  mode5_fsm.add_transition(&two_digits_entered, &three_digits_entered, NUMBER_PRESSED, &store_racer_number);
  mode5_fsm.add_transition(&two_digits_entered, &ready_for_sensor, ACCEPT, NULL);

  mode5_fsm.add_transition(&three_digits_entered, &initial, DELETE, &clear_racer_number);
  mode5_fsm.add_transition(&three_digits_entered, &initial, NUMBER_PRESSED, &clear_racer_number); // TODO: add better error transition?
  mode5_fsm.add_transition(&three_digits_entered, &ready_for_sensor, ACCEPT, NULL);

  mode5_fsm.add_transition(&ready_for_sensor, &initial, SENSOR, &sensor_triggered);
  mode5_fsm.add_transition(&ready_for_sensor, &initial, DELETE, NULL);
}

void mode5_setup() {


  sensor.attach_interrupt();
  // States:
  // INITIAL
  // ONE_DIGIT_ENTERED
  // TWO_DIGITS_ENTERED
  // THREE_DIGITS_ENTERED
  // READY_FOR_SENSOR

  // Transitions
  // INITIAL:
  // - 0-9 -> ONE_DIGIT_ENTERED
  // - C+* -> (Cancel Previous Data AND) INITIAL
  // - SENSOR -> Beep
//   CANCELLING:
//   - <NONE>
  // ONE_DIGIT_ENTERED:
  // - 0-9 -> TWO_DIGITS_ENTERED
  // - A -> ACCEPTING
  // - D -> INITIAL
  // TWO_DIGITS_ENTERED:
  // - 0-9 -> THREE_DIGITS_ENTERED
  // - A -> ACCEPTING
  // - D -> INITIAL
  // THREE_DIGITS_ENTERED:
  // - 0-9 -> ERROR
  // - A -> ACCEPTING
  // - D -> INITIAL
//   ACCEPTING:
//   - ACCEPTED -> READY_FOR_SENSOR
  // READY_FOR_SENSOR:
  // - SENSOR -> RECORD
  // - D -> ERROR
//   RECORD:
  

  // Entry/Exit
  // INITIAL:
  // - Clear the racer number
//   CANCELLING:
//   - On Entry -> Cancel previous, trigger CANCELED
  // ONE_DIGIT_ENTERED
  // - On Entry -> Store current keypress
  // TWO_DIGITS_ENTERED
  // - On Entry -> Store current keypress
  // THREE_DIGITS_ENTERED
  // - On Entry -> Store current keypress
//   ACCEPTING
//   - On Entry -> Store the current racer number, trigger ACCEPTED
  // READY_FOR_SENSOR
  // - On Entry -> Success Music
//   ERROR
//   - ON Entry -> Display Error, Beep, trigger START
//   RECORD:
//   - ON entry -> BEEP, DISPLAY AND RECORD trigger START
}
void mode5_loop() {
  mode5_fsm.run_machine();
}

void mode5_teardown() {
  sensor.detach_interrupt();
}
