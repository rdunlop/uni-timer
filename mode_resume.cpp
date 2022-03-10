#include "uni_keypad.h"
#include "uni_gps.h"
#include "uni_display.h"
#include "uni_buzzer.h"
#include "modes.h"

extern UniKeypad keypad;
extern UniGps gps;
extern UniDisplay display;

#include <Fsm.h>

/*********************************************************************************** */
//### Mode Resume - GPS lock requirement before entering Mode 5 or Mode 6
//
//- Displays a moving pattern while waiting for GPS lock
//- Once GPS lock, moves into the target mode
//

// *****************************************************
// Mode Resume FSM

void mode_resume_setup() {
  display.clear();
}
void mode_resume_loop() {
  gps.readData();
  if (gps.lock()) {
    // mode_fsm.trigger(MODE_5);
  } else {
    display.waiting(false);
  }
}

void mode_resume_teardown() {
}
