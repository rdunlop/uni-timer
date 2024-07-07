#include "uni_keypad.h"
#include "uni_display.h"
#include "uni_buzzer.h"
#include "uni_config.h"
#include "uni_radio.h"
#include "modes.h"
#include "recording.h"
#include "accurate_timing.h"

extern UniKeypad keypad;
extern UniDisplay display;
extern UniBuzzer buzzer;
extern UniConfig config;
extern UniRadio radio;

#include <Fsm.h>

/*********************************************************************************** */
//### Mode 7 - Race Results (Near Finish Line)
//
//- Type in a racer number, and press A, and it will try to display the most recent race result
//- Press "C" to clear entry
//- Press B to cycle through the stored results
//

// *****************************************************
// Mode 5 FSM

// Data

void mode7_initial_check();
void mode7_digit_check();
void mode7_display();
void mode7_clear_display();
void mode7_receive_radio();

State mode7_initial(NULL, &mode7_initial_check, NULL);
State mode7_digits_entered(NULL, &mode7_digit_check, NULL);
State mode7_display_result(&mode7_display, &mode7_digit_check, &mode7_clear_display);
bool fsm_7_transition_setup_complete = false;

Fsm mode7_fsm(&mode7_initial);

#define NUMBER_PRESSED 1
#define DELETE 2
#define ACCEPT 3
#define SENSOR 5 // we use this for RADIO message receipt triggers

void mode7_initial_check() {
  if (radio.messageAvailable()) {
    mode7_fsm.trigger(SENSOR);
  }

  char last_key_pressed = keypad.readChar();
  if (keypad.isDigit(last_key_pressed)) {
    mode7_fsm.trigger(NUMBER_PRESSED);
  }
#ifdef FSM_DEBUG
  Serial.println("Initial Check ");
#endif
}

void mode7_digit_check() {
  if (radio.messageAvailable()) {
    mode7_fsm.trigger(SENSOR);
  }

  // - 0-9 -> TWO_DIGITS_ENTERED or THREE_DIGITS_ENTERED
  // - A -> ACCEPTING
  // - C -> INITIAL
  char last_key_pressed = keypad.readChar();
  if (keypad.isDigit(last_key_pressed)) {
    if (maximum_digits_racer_number()) {
      mode7_fsm.trigger(DELETE);
    } else {
      mode7_fsm.trigger(NUMBER_PRESSED);
    }
  } else if (last_key_pressed == 'A') {
    mode7_fsm.trigger(ACCEPT);
  } else if (last_key_pressed == 'C') {
    mode7_fsm.trigger(DELETE);
    log("CLEARED RACER NUMBER");
  }
#ifdef FSM_DEBUG
  Serial.println("Digit Check");
#endif
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

void mode7_fsm_setup() {
  mode7_fsm.add_transition(&mode7_initial, &mode7_digits_entered, NUMBER_PRESSED, &store_racer_number);
  mode7_fsm.add_transition(&mode7_initial, &mode7_initial, SENSOR, &mode7_receive_radio);
  
  mode7_fsm.add_transition(&mode7_digits_entered, &mode7_initial, DELETE, &clear_racer_number);
  mode7_fsm.add_transition(&mode7_digits_entered, &mode7_digits_entered, NUMBER_PRESSED, &store_racer_number);
  mode7_fsm.add_transition(&mode7_digits_entered, &mode7_display_result, ACCEPT, NULL);
  mode7_fsm.add_transition(&mode7_digits_entered, &mode7_digits_entered, SENSOR, &mode7_receive_radio);

  // make this a timed transition?
  mode7_fsm.add_transition(&mode7_display_result, &mode7_initial, DELETE, &clear_racer_number);
  mode7_fsm.add_timed_transition(&mode7_display_result, &mode7_initial, 10000, &clear_racer_number); // 10 seconds
  mode7_fsm.add_transition(&mode7_display_result, &mode7_display_result, SENSOR, &mode7_receive_radio);
}

void mode7_setup() {
  if (!fsm_7_transition_setup_complete) {
    mode7_fsm_setup();
    fsm_7_transition_setup_complete = true;
  }
  Serial.println("starting mode 7");
  display.clear();
}

void mode7_loop() {
  mode7_fsm.run_machine();
}

void mode7_teardown() {
  // none
}

// DISPLAY Received data

typedef struct {
  uint16_t racer_number;
  uint32_t start_millis;
  uint8_t start_fault;
  uint32_t end_millis;
} RxResult;


#define MAX_RECENT_RESULTS 30
RxResult lookup_table[MAX_RECENT_RESULTS];
int rx_results_count = 0;
// Oldest result is in location 0

// stores start-line data for a racer
// removing the oldest entry, if needed
void rxStore(uint16_t racer_number, uint32_t time, uint8_t fault) {
  Serial.println("Storing ");
  Serial.println(racer_number);
  Serial.println(" - ");
  Serial.println(time);
  // if this racer number has an entry, update that entry
  for (int i = 0; i < rx_results_count; i++) {
    if (lookup_table[i].racer_number == racer_number) {
      lookup_table[i].start_millis = time;
      lookup_table[i].start_fault = fault;
      return;
    }
  }

  // the racer number was not found
  // if there is free space, store it at the end
  if (rx_results_count < MAX_RECENT_RESULTS) {
    lookup_table[rx_results_count].racer_number = racer_number;
    lookup_table[rx_results_count].start_millis = time;
    lookup_table[rx_results_count].start_fault = fault;
    rx_results_count += 1;
    return;
  }

  // the racer number was not found, and there is no space
  // copy all entries up, and overwrite the last entry
  for (int i = 1; i < (MAX_RECENT_RESULTS - 1); i++) {
    memcpy(&lookup_table[i - 1], &lookup_table[i], sizeof(lookup_table[i]));
  }
  lookup_table[MAX_RECENT_RESULTS - 1].racer_number = racer_number;
  lookup_table[MAX_RECENT_RESULTS - 1].start_millis = time;
  lookup_table[MAX_RECENT_RESULTS - 1].start_fault = fault;
}

void displayRx() {
  Serial.println("RX");
  for (int i = 0; i < MAX_RECENT_RESULTS; i++) {
    Serial.print(i);
    Serial.print(": ");
    Serial.print(lookup_table[i].racer_number);
    Serial.print(" - ");
    Serial.print(lookup_table[i].start_millis);
    Serial.print(" - ");
    Serial.print(lookup_table[i].start_fault);
    Serial.println();
  }
}

// show the entered racer number's data
void mode7_display() {
  char racer[15], line2[16];
  snprintf(racer, 15, "racer: %d", racer_number());
  bool found = false;
  for (int i = 0; i < rx_results_count; i++) {
    if (lookup_table[i].racer_number == racer_number()) {
      sprintf(line2, "%ld", lookup_table[i].start_millis);
      found = true;
      break;
    }
  }
  if (!found) {
    sprintf(line2, "No data");
  }
  display.print(racer, line2);
}

void mode7_clear_display() {
  display.clear();
}

// process that the radio has new data for us
void mode7_receive_radio() {
  // Should be a message for us now
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
  if (radio.receive(buf, &len))
  {
    uint16_t racer_number;
    uint16_t min, sec, mil;
    uint8_t fault;
    // the sscanf format MUST have the correct size of storage (%hhd, vs %hd, vs %d)
    if (sscanf((char *)buf, "%hd,,%hd,%hd,%hd,%hhd", &racer_number, &min, &sec, &mil, &fault) == 5) {
      // all matched
      Serial.println("Successfully read message");
      rxStore(racer_number, (((min * 60) + sec) * 1000) + mil, fault);
    } else {
      Serial.println("Error parsing message");
    }
    // return true;
  } else {
    Serial.println("recv failed");
    // return false;
  }
}
