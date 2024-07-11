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
void mode7_store_result_display_time();
uint32_t _result_display_time = 0;

State mode7_initial(&mode7_clear_display, &mode7_initial_check, NULL);
State mode7_digits_entered(NULL, &mode7_digit_check, NULL);
State mode7_display_result(&mode7_display, &mode7_digit_check, NULL);
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
}

// store the time that we entered the display-state
// so that we can auto-transition out
void mode7_store_result_display_time() {
  _result_display_time = millis();
}

void mode7_digit_check() {
  if (_result_display_time != 0 && (millis() - _result_display_time) > 10000) {
    mode7_fsm.trigger(DELETE);
  }
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
}

/*
 * Possible Actions:
 * Sensor
 * Number
 * C
 * A
 * 
 * Possible States:
 * INITIAL
 * DIGITS
 */

void mode7_fsm_setup() {
  mode7_fsm.add_transition(&mode7_initial, &mode7_digits_entered, NUMBER_PRESSED, &store_racer_number);
  mode7_fsm.add_transition(&mode7_initial, &mode7_initial, SENSOR, &mode7_receive_radio);
  
  mode7_fsm.add_transition(&mode7_digits_entered, &mode7_initial, DELETE, &clear_racer_number);
  mode7_fsm.add_transition(&mode7_digits_entered, &mode7_digits_entered, NUMBER_PRESSED, &store_racer_number);
  mode7_fsm.add_transition(&mode7_digits_entered, &mode7_display_result, ACCEPT, &mode7_store_result_display_time);
  mode7_fsm.add_transition(&mode7_digits_entered, &mode7_digits_entered, SENSOR, &mode7_receive_radio);

  // make this a timed transition?
  mode7_fsm.add_transition(&mode7_display_result, &mode7_initial, DELETE, &clear_racer_number);
  mode7_fsm.add_transition(&mode7_display_result, &mode7_display_result, SENSOR, &mode7_receive_radio);
}

void mode7_setup() {
  if (!fsm_7_transition_setup_complete) {
    mode7_fsm_setup();
    fsm_7_transition_setup_complete = true;
  }
  Serial.println("starting mode 7");
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
void rxStore(bool start, uint16_t racer_number, uint32_t time, uint8_t fault) {
  Serial.println("Storing ");
  Serial.println(racer_number);
  Serial.println(" - ");
  Serial.println(time);
  // if this racer number has an entry, update that entry
  for (int i = 0; i < rx_results_count; i++) {
    if (lookup_table[i].racer_number == racer_number) {
      if (start) {
        lookup_table[i].start_millis = time;
        lookup_table[i].start_fault = fault;
      } else {
        lookup_table[i].end_millis = time;
      }
      return;
    }
  }

  // the racer number was not found
  // if there is free space, store it at the end
  if (rx_results_count < MAX_RECENT_RESULTS) {
    lookup_table[rx_results_count].racer_number = racer_number;
    if (start) {
      lookup_table[rx_results_count].start_millis = time;
      lookup_table[rx_results_count].start_fault = fault;
    } else {
      lookup_table[rx_results_count].end_millis = time;
    }
    rx_results_count += 1;
    return;
  }

  // the racer number was not found, and there is no space
  // copy all entries up, and overwrite the last entry
  for (int i = 1; i < MAX_RECENT_RESULTS; i++) {
    memcpy(&lookup_table[i - 1], &lookup_table[i], sizeof(lookup_table[i]));
  }
  memset(&lookup_table[MAX_RECENT_RESULTS - 1], 0, sizeof(lookup_table[MAX_RECENT_RESULTS - 1]));
  lookup_table[MAX_RECENT_RESULTS - 1].racer_number = racer_number;
  if (start) {
    lookup_table[MAX_RECENT_RESULTS - 1].start_millis = time;
    lookup_table[MAX_RECENT_RESULTS - 1].start_fault = fault;
  } else {
    lookup_table[MAX_RECENT_RESULTS - 1].end_millis = time;
  }
}

void displayRx() {
  Serial.println("RX");
  Serial.print("Results count: ");
  Serial.println(rx_results_count);
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
  snprintf(racer, 15, "ID: %d", racer_number());
  bool found = false;
  for (int i = 0; i < rx_results_count; i++) {
    if (lookup_table[i].racer_number == racer_number()) {
      if (lookup_table[i].start_millis != 0 && lookup_table[i].end_millis != 0) {
        // both values exist, do some math
        if (lookup_table[i].start_millis < lookup_table[i].end_millis) {
          // valid
          uint32_t diff =  lookup_table[i].end_millis - lookup_table[i].start_millis;
          uint16_t min, sec, thou;
          thou = diff % 1000;
          sec = (diff / 1000) % 60;
          min = ((diff / 1000) - sec) / 60;
          snprintf(line2, 16, "%02hd:%02hd.%03hd", min, sec, thou);
        } else {
          // have newer start than end
          snprintf(line2, 16, "No End data");
        }
      } else {
        snprintf(line2, 16, "Only 1 time");
      }
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
  _result_display_time = 0;
  display.print("Enter ID", "Then Press A");
}

// process that the radio has new data for us
void mode7_receive_radio() {
  // Should be a message for us now
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
  if (radio.receive(buf, &len))
  {
    char logbuf[RH_RF95_MAX_MESSAGE_LEN + 1];
    snprintf(logbuf, RH_RF95_MAX_MESSAGE_LEN, "%s", buf);
    log(logbuf);
    buzzer.beep();
    uint16_t racer_number;
    uint16_t min, sec, mil;
    uint8_t fault;
    char start_or_end;
    // the sscanf format MUST have the correct size of storage (%hhd, vs %hd, vs %d)
    if (sscanf((char *)buf, "%c,%hd,,%hd,%hd,%hd,%hhd", &start_or_end, &racer_number, &min, &sec, &mil, &fault) == 6) {
      // all matched
      Serial.println("Successfully read message");
      rxStore(start_or_end == 'S', racer_number, (((min * 60) + sec) * 1000) + mil, fault);
      displayRx();
    } else {
      Serial.println("Error parsing message");
    }
  } else {
    Serial.println("recv failed");
  }
}
