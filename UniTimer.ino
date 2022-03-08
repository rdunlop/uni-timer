/* ****************************************************************************************** */
// UniTimer
//
// This application interacts with an optical sensor device, and stores the results
// on an SD card as well as displays to a 7-segment display
//
// Expected Hardware Components
// - SENSOR - 10v hardware optical Sensor
// - GPS - GPS Sensor, for setting accurate time signal
// - DISPLAY - 7 Segment display
// - BUZZER - Piezo buzzer
// - SD - MicroSD Storage card
//
//
// NOTES:
// [1] The GPS is used to know the absolute time.
//     Based on https://wyolum.com/syncing-arduino-with-gps-time/.
//     Whenever we have GPS lock, we keep track of the offset from micros() for the GPS time
//     and we use that offset whenever we are printing the time.
// [2] Modified the Keypad_I2C library so that it provides the hardware address of the wire bus.
//     I had to change the Keypad_I2C.h file:
//         Keypad_I2C(char* userKeymap, byte* row, byte* col, byte numRows, byte numCols, byte address, byte width = 1) :
//         #if defined(__arm__) && defined(TEENSYDUINO)
//         Keypad(userKeymap, row, col, numRows, numCols), TwoWire(address, i2c0_hardware)
//         #else
//         Keypad(userKeymap, row, col, numRows, numCols)
//         #endif
//         { i2caddr = address; i2cwidth = width;}
//     There may be a way to specify a default-constructor for TwoWire, so that I don't have to do this?
/* ****************************************************************************************** */

// https://www.bastelgarage.ch/index.php?route=extension/d_blog_module/post&post_id=8
// https://github.com/joeyoung/arduino_keypads/blob/master/Keypad_I2C/Keypad_I2C.h

/* ************************* Capabilities flags ******************************************* */
/* Set these flags to enable certain combinations of components */
#define ENABLE_GPS
#define ENABLE_DISPLAY
#define ENABLE_KEYPAD
#define ENABLE_SD
#define ENABLE_SENSOR
#define ENABLE_BUZZER

/* *********************** Includes *********************************** */
// - SENSOR
// - DISPLAY
#ifdef ENABLE_DISPLAY
#include "uni_display.h"
#endif
#ifdef ENABLE_GPS
#include "uni_gps.h"
#endif
// - KEYPAD
#ifdef ENABLE_KEYPAD
#include "uni_keypad.h"
#endif
// - SD Card
#ifdef ENABLE_SD
#include "uni_sd.h"
#endif
// - BUZZER
#ifdef ENABLE_BUZZER
#include "uni_buzzer.h"
#endif
#ifdef ENABLE_SENSOR
#include "uni_sensor.h"
#endif

#include "modes.h"
#include "recording.h"
#include "accurate_timing.h"

/* *************************** (Defining Global Variables) ************************** */
// - SENSOR
#define SENSOR_DIGITAL_INPUT 5
// - GPS
#define GPS_PPS_DIGITAL_INPUT 2
#define GPS_DIGITAL_OUTPUT 9 // hardware serial #2
#define GPS_DIGITAL_INPUT 10 // hardware serial #2
// - DISPLAY
#define DISPLAY_I2CADDR 0x70
// - KEYPAD
#define KEYPAD_COLUMN_WIRE_1 23
#define KEYPAD_COLUMN_WIRE_2 22
#define KEYPAD_COLUMN_WIRE_3 21
#define KEYPAD_COLUMN_WIRE_4 20
#define KEYPAD_ROW_WIRE_1 17
#define KEYPAD_ROW_WIRE_2 16
#define KEYPAD_ROW_WIRE_3 15
#define KEYPAD_ROW_WIRE_4 14
// - SD Card
#define SD_SPI_CHIP_SELECT_OUTPUT 6
#define SD_SPI_MOSI_INPUT 11
#define SD_SPI_MISO_INPUT 12
#define SD_SPI_CLK_OUTPUT 13
// - BUZZER
#define BUZZER_DIGITAL_OUTPUT 4

#define MODE_OFFSET 100
#define MODE_1 101
#define MODE_2 102
#define MODE_3 103
#define MODE_4 104
#define MODE_5 105
#define MODE_6 106
#define MODE_RESUME 107

/* ************************** Initialization ******************* */

// KEYPAD --------------------------------------------
#ifdef ENABLE_KEYPAD
UniKeypad keypad(
  KEYPAD_ROW_WIRE_1,
  KEYPAD_ROW_WIRE_2,
  KEYPAD_ROW_WIRE_3,
  KEYPAD_ROW_WIRE_4,
  KEYPAD_COLUMN_WIRE_1,
  KEYPAD_COLUMN_WIRE_2,
  KEYPAD_COLUMN_WIRE_3,
  KEYPAD_COLUMN_WIRE_4
);
UniKeypad modeKeypad(
  KEYPAD_ROW_WIRE_1,
  KEYPAD_ROW_WIRE_2,
  KEYPAD_ROW_WIRE_3,
  KEYPAD_ROW_WIRE_4,
  KEYPAD_COLUMN_WIRE_1,
  KEYPAD_COLUMN_WIRE_2,
  KEYPAD_COLUMN_WIRE_3,
  KEYPAD_COLUMN_WIRE_4
);
#endif

// SD
#ifdef ENABLE_SD
UniSd sd(
  SD_SPI_CHIP_SELECT_OUTPUT,
  SD_SPI_MOSI_INPUT,
  SD_SPI_MISO_INPUT,
  SD_SPI_CLK_OUTPUT);
#endif

#ifdef ENABLE_DISPLAY
UniDisplay display(DISPLAY_I2CADDR);
#endif

#ifdef ENABLE_GPS
UniGps gps(GPS_PPS_DIGITAL_INPUT);
#endif

#ifdef ENABLE_BUZZER
UniBuzzer buzzer(BUZZER_DIGITAL_OUTPUT);
#endif

#ifdef ENABLE_SENSOR
UniSensor sensor(SENSOR_DIGITAL_INPUT);
#endif

// NEW HEADER FILE
void clear_display();
void date_callback(byte *hour, byte *minute, byte *second);

// ****************** MODE FSM ***************************
#include <Fsm.h>

// *****************************************************
State mode0(&mode0_run, NULL, NULL);
State mode1(&clear_display, &mode1_loop, NULL);
State mode2(&clear_display, &mode2_loop, NULL);
State mode3(&clear_display, &mode3_loop, NULL);
State mode4(&mode4_setup, &mode4_loop, NULL);
State mode5(&mode5_setup, &mode5_loop, &mode5_teardown);
State mode6(&mode6_setup, &mode6_loop, &mode6_teardown);
State mode_resume(&mode_resume_setup, &mode_resume_loop, &mode_resume_teardown);


Fsm mode_fsm(&mode0);
// *******************************************************************

/******** ***********************************(set up)*** *************** **********************/
void setup () {
  // Common
  Serial.begin(115200);
  pinMode (LED_BUILTIN, OUTPUT);

  // SENSOR
#ifdef ENABLE_SENSOR
  sensor.setup(&sensor_interrupt);
#endif

  // DISPLAY
#ifdef ENABLE_DISPLAY
  display.setup();
#endif

  delay(2000); // wait for serial to connect before starting
  Serial.println("Starting");

  // KEYPAD
#ifdef ENABLE_KEYPAD
  keypad.setup();
  modeKeypad.setup();
#endif

  // GPS
#ifdef ENABLE_GPS
  gps.setup(&pps_interrupt);
#endif

#ifdef ENABLE_BUZZER
  buzzer.setup();
#endif

#ifdef ENABLE_SD
  // sd.setup();
#endif

  register_date_callback(date_callback);
  setup_fsm();
}

void date_callback(byte *hour, byte *minute, byte *second) {
  if (gps.current_time(hour, minute, second)) {
    Serial.println("OK");
  } else {
    Serial.println("Not OK");
  }
}


// MODE Selection FSM
void loop() {
  mode_fsm.run_machine();
  
  gps.readData();
  checkForModeSelection();
}


void setup_fsm() {
  mode_fsm.add_transition(&mode0, &mode_resume, MODE_RESUME, NULL); // Able to go to RESUME mode from POST
  mode_fsm.add_transition(&mode0, &mode1, MODE_1, NULL); // Able to go to MODE 1 mode from POST

  // Set up transitions between each possible state and each other state, based on MODE_1, MODE_2, etc triggers.
  State *mode_states[] = { &mode1, &mode2, &mode3, &mode4, &mode_resume};
  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 6; j++) {
      if (j == i) continue; // Don't need to transition from state to same state.
      mode_fsm.add_transition(mode_states[i], mode_states[j], MODE_OFFSET + j + 1, NULL);
    }
  }
  /* Can transition from RESUME to 1, 5 or 6 */
  mode_fsm.add_transition(&mode_resume, &mode1, MODE_1, NULL);
  mode_fsm.add_transition(&mode_resume, &mode5, MODE_5, NULL);
  mode_fsm.add_transition(&mode_resume, &mode6, MODE_6, NULL);
}

void clear_display() { 
  display.clear();
}


// Variables
int _mode = 1;
int _new_mode = -1;

// POST - Check systems, and display Good or Bad on the display
void mode0_run() {
  bool success = true;

  buzzer.beep();
  // Show 88:88
  display.all();
  delay(1000);

#ifdef ENABLE_SD
  display.sd();
  delay(1000);
  if (sd.status()) {
    Serial.println("SD Card OK");
    display.good();
    buzzer.success();
  } else {
    Serial.println("SD Card Error");
    success = false;
    display.bad();
    buzzer.failure();
  }
  delay(1000);
#endif

  // TODO: Check GPS
  display.gps();
  delay(1000);
  gps.readData();
  // Any better option?
  buzzer.success();
  delay(1000);

  if (success) {
    Serial.println("All systems Good");
    display.good();
    delay(1000);
    mode_fsm.trigger(MODE_RESUME);
    _new_mode = 7; // simulate user transition to Mode Resume
  } else {
    Serial.println("*************** Init Problem");
    display.bad();
    delay(1000);
    mode_fsm.trigger(MODE_1);
    _new_mode = 1; // simulate user transition to Mode 1
  }
}

/************************************* (main program) ********* *****************************/

// ------------------------------------------

// Check to see if a new mode is selected
void checkForModeSelection() {
  // Only switch to the new mode after all keys are pressed
  if (_new_mode != _mode && !modeKeypad.anyKeyPressed()) {
    Serial.print("new mode: ");
    Serial.println(_new_mode);
    mode_fsm.trigger(MODE_OFFSET + _new_mode); // trigger MODE_1, MODE_2, etc
    _mode = _new_mode;
  }
  
  // Detect star AND number 1-6 pressed at same time
  if (modeKeypad.newKeyPressed()) {
    Serial.println("NEW KEY");
    if (modeKeypad.keyPressed('*')) {
      Serial.println("* is pressed");
      if (modeKeypad.keyPressed('1')) _new_mode = 1;
      if (modeKeypad.keyPressed('2')) _new_mode = 2;
      if (modeKeypad.keyPressed('3')) _new_mode = 3;
      if (modeKeypad.keyPressed('4')) _new_mode = 4;
      if (modeKeypad.keyPressed('5')) _new_mode = 5;
      if (modeKeypad.keyPressed('6')) _new_mode = 6;
    }
  }
}
