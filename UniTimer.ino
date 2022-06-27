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
//     Whenever we have GPS lock, we keep track of the offset from millis() for the GPS time
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

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}

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
#include "mode_fsm.h"
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
// #define SD_SPI_MOSI_INPUT 11 // unused
// #define SD_SPI_MISO_INPUT 12
// #define SD_SPI_CLK_OUTPUT 13
// - BUZZER
#define BUZZER_DIGITAL_OUTPUT 4


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
UniSd sd(SD_SPI_CHIP_SELECT_OUTPUT);
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

// CONFIG MANAGEMENT
UniConfig config; // No arguments for constructor, hence, no parentheses

// NEW HEADER FILE
void clear_display();

// ****************** MODE FSM ***************************
#include <Fsm.h>

// *****************************************************
State mode0(&mode0_run, NULL, NULL);
State mode1(&clear_display, &mode1_loop, NULL);
State mode2(&clear_display, &mode2_loop, NULL);
State mode3(&clear_display, &mode3_loop, NULL);
State mode4(&mode4_setup, &mode4_loop, &mode4_teardown);
State mode5(&mode5_setup, &mode5_loop, &mode5_teardown);
State mode6(&mode6_setup, &mode6_loop, &mode6_teardown);
State mode_resume_5(&mode_resume_setup, &mode_resume_loop, &mode_resume_teardown);
State mode_resume_6(&mode_resume_setup, &mode_resume_loop, &mode_resume_teardown);


Fsm mode_fsm(&mode0);
// *******************************************************************

/******** ***********************************(set up)*** *************** **********************/
void setup () {
  pinMode (LED_BUILTIN, OUTPUT);

  // Common
  Serial.begin(115200);
  delay(2000); // wait for serial to connect before starting
  Serial.println("Starting");

  // SENSOR
#ifdef ENABLE_SENSOR
  sensor.setup(&sensor_interrupt);
#endif

  // DISPLAY
#ifdef ENABLE_DISPLAY
  display.setup();
#endif

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
  sd.setup();
#endif

  config.setup();
  if (config.loadedFromDefault()) {
    Serial.println("Config File not found, loaded defaults");
    buzzer.failure();
  } else {
    Serial.println("Config Read Success");
    buzzer.success();
  }

  setup_fsm();
  memset(recentRacer, 0, sizeof(recentRacer));
  memset(recentResult, 0, sizeof(recentResult));
}

uint32_t last_memory_output_time = 0;

void printMemoryPeriodically() {
  // if millis() or timer wraps around, we'll just reset it
  if (last_memory_output_time > millis())  last_memory_output_time = millis();
  // approximately every 2 seconds or so, print out the current GPS stats
  if (millis() - last_memory_output_time > 10000) {
    last_memory_output_time = millis(); // reset the timer

    Serial.println(F("Memory Free"));
    Serial.println(freeMemory());
  }
}

// MODE Selection FSM
void loop() {
  mode_fsm.run_machine();
  
  gps.readData();
  checkForModeSelection();
  printMemoryPeriodically();
}

void setup_fsm() {
  // Able to go to RESUME mode from POST
  mode_fsm.add_transition(&mode0, &mode_resume_5, MODE_RESUME_5, NULL);
  mode_fsm.add_transition(&mode0, &mode_resume_6, MODE_RESUME_6, NULL);
  mode_fsm.add_transition(&mode0, &mode1, MODE_1, NULL); // Able to go to MODE 1 mode from POST

  // Set up transitions between mode1 and all other possible states
  State *mode_states[] = { &mode2, &mode3, &mode4, &mode_resume_5, &mode_resume_6};
  for (int i = 0; i < 5; i++) {
    mode_fsm.add_transition(&mode1, mode_states[i], MODE_OFFSET + i + 2, NULL);
    mode_fsm.add_transition(mode_states[i], &mode1, MODE_1, NULL);
  }
  /* Can transition from RESUME_5 to 5 */
  mode_fsm.add_transition(&mode_resume_5, &mode5, MODE_GPS_LOCK, NULL);
  mode_fsm.add_transition(&mode5, &mode1, MODE_1, NULL);
  /* Can transition from RESUME_6 to 6 */
  mode_fsm.add_transition(&mode_resume_6, &mode6, MODE_GPS_LOCK, NULL);
  mode_fsm.add_transition(&mode6, &mode1, MODE_1, NULL);
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
  if (gps.detected()) {
    Serial.println("GPS available");
    display.good();
    buzzer.success();
  } else {
    Serial.println("GPS unavailable");
    display.bad();
    buzzer.failure();
  }
  delay(1000);

  display.all();
  delay(1000);
  if (success) {
    Serial.println("All systems Good");
    display.good();
    delay(1000);
    int target_mode = MODE_OFFSET + config.mode();
    if (target_mode == MODE_5) {
      mode_fsm.trigger(MODE_1);
      mode_fsm.trigger(MODE_RESUME_5);
      _new_mode = MODE_RESUME_5 - MODE_OFFSET; // simulate user transition to Mode Resume
    } else if (target_mode == MODE_6) {
      mode_fsm.trigger(MODE_1);
      mode_fsm.trigger(MODE_RESUME_6);
      _new_mode = MODE_RESUME_6 - MODE_OFFSET; // simulate user transition to Mode Resume
    } else {
      mode_fsm.trigger(MODE_1);
      mode_fsm.trigger(target_mode);
      _new_mode = config.mode();
    }
    Serial.println("Resuming");
    Serial.println(_new_mode);
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
    config.setMode(_new_mode);
    mode_fsm.trigger(MODE_1); // go to mode 1 before any other mode
    mode_fsm.trigger(MODE_OFFSET + _new_mode); // trigger MODE_1, MODE_2, etc
    _mode = _new_mode;
  }
  
  if (modeKeypad.newKeyPressed()) {
    Serial.println("NEW KEY");
    // Detect star AND number 1-6 pressed at same time
    // Switches mode
    if (modeKeypad.keyPressed('*')) {
      Serial.println("* is pressed");
      if (modeKeypad.keyPressed('1')) _new_mode = 1;
      if (modeKeypad.keyPressed('2')) _new_mode = 2;
      if (modeKeypad.keyPressed('3')) _new_mode = 3;
      if (modeKeypad.keyPressed('4')) _new_mode = 4;
      if (modeKeypad.keyPressed('5')) _new_mode = 5;
      if (modeKeypad.keyPressed('6')) _new_mode = 6;
    }

    // Detect 0 and number 1-9 pressed at same time
    // displays recent results
    if (modeKeypad.keyPressed('0')) {
      TimeResult *data;
      int index = -1;
      // display the ssmm (2 seconds, and 2 digits of milliseconds)
      // display.showNumber((data.second * 100) + (data.millisecond /10));
      // display the milliseconds

      Serial.println("0 is pressed");
      if (modeKeypad.keyPressed('1')) index = 0;
      if (modeKeypad.keyPressed('2')) index = 1;
      if (modeKeypad.keyPressed('3')) index = 2;
      if (modeKeypad.keyPressed('4')) index = 3;
      if (modeKeypad.keyPressed('5')) index = 4;
      if (modeKeypad.keyPressed('6')) index = 5;
      if (modeKeypad.keyPressed('7')) index = 6;
      if (modeKeypad.keyPressed('8')) index = 7;
      if (modeKeypad.keyPressed('9')) index = 8;

      if (index != -1) {
        data = &recentResult[index];
        display.showNumber(data->minute);
        delay(500);
        display.showNumber(data->second);
        delay(500);
        display.showNumber(data->millisecond);
        delay(500);
      }
    }
  }
}
