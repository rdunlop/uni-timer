/* ****************************************************************************************** */
// UniTimer
//
// This application interacts with an optical sensor device, and prints the results
// to a thermal printer as well as displays to a 7-segment display
//
// Expected Hardware Components
// - SENSOR - Sensor
// - GPS - GPS Sensor, for setting accurate time signal
// - DISPLAY - 7 Segment display
// - KEYPAD_EXPANSION - I2C expansion board, with keypad connected to it
// - BUZZER - Piezo buzzer
// - BUTTON - Input button
// - SD - MicroSD Storage card
//
// Needed Libraries
// - Download and provide https://github.com/adafruit/SD in the SD folder. (this replaces the SD library included by the GPS library
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
#define ENABLE_PRINTER
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
// - PRINTER
#ifdef ENABLE_PRINTER
#include "uni_printer.h"
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
// - BUTTON

#include "modes.h"

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
// - PRINTER
#define PRINTER_DIGITAL_OUTPUT 8 // Arduino transmit  YELLOW WIRE  labeled RX on printer
#define PRINTER_DIGITAL_INPUT 7 // Arduino receive   GREEN WIRE   labeled TX on printer
// - SD Card
#define SD_SPI_CHIP_SELECT_OUTPUT 6
#define SD_SPI_MOSI_INPUT 11
#define SD_SPI_MISO_INPUT 12
#define SD_SPI_CLK_OUTPUT 13
// - BUZZER
#define BUZZER_DIGITAL_OUTPUT 4
// - BUTTON
#define BUTTON_DIGITAL_INPUT 25 // unused


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

// PRINTER -------------------------------------
#ifdef ENABLE_PRINTER
UniPrinter printer(PRINTER_DIGITAL_INPUT, PRINTER_DIGITAL_OUTPUT);
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

/******** ***********************************(set up)*** *************** **********************/
void setup () {
  // Common
  Serial.begin(115200);
  pinMode (LED_BUILTIN, OUTPUT);

  // SENSOR
#ifdef ENABLE_SENSOR
  sensor.setup();
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
  gps.setup();
#endif

  // PRINTER
#ifdef ENABLE_PRINTER
  printer.setup();
#endif

#ifdef ENABLE_BUZZER
  buzzer.setup();
#endif

#ifdef ENABLE_SD
  sd.setup();
#endif

  setup_fsm();
}
// Variables
int _mode = 1;
int _new_mode = -1;

// Check systems, and display Good or Bad on the display
void mode0_run() {
  bool success = true;

  // Show 88:88
  display.all();
  
  if (printer.hasPaper()) {
    Serial.println("printer has paper");
  } else {
    Serial.println("printer has no paper");
    success = false;
  }

  if (sd.status()) {
    Serial.println("SD Card OK");
  } else {
    Serial.println("SD Card Error");
    success = false;
  }

  // TODO: Check GPS


  // Wait 2 seconds
  delay(2000);

  if (success) {
    Serial.println("All systems Good");
    display.good();
  } else {
    Serial.println("*************** Init Problem");
    display.bad();
  }
  // wait 1 second
  delay(1000);
}

/************************************* (main program) ********* *****************************/

bool currentTime(unsigned long current_micros, char *output) {
  int hour, minute, second, millisecond;
  bool res = gps.current_time(current_micros, &hour, &minute, &second, &millisecond);
  Serial.print("Res: ");
  Serial.println(res);
  sprintf(output, "%02d:%02d:%02d.%03d", hour, minute, second, millisecond);
  return true;
}


//### Mode 2 - GPS/Printer/SD Test
//
//- If you press A, it will show the GPS time, and beep positively.
//- If you press B, it will show print a test line on the printer.
//- If you press C, it will test writing/reading from the SD card, and display either 6ood or bAd
unsigned long gps_millis = 0;
char last_key2 = NO_KEY;
char last_key4 = NO_KEY;
void mode2_loop() {
  if (gps_millis == 0 || (millis() - gps_millis > 100)) {
    gps_millis = millis();
    gps.printGPSDate();
  }
  char key = keypad.readChar();
  if (key != NO_KEY) {
    if (key != last_key2) {
      // New Keypress
      int keynum = keypad.intFromChar(key);
      if (keynum == 17) {
        // A
        int hour, minute, second;
        gps.printPeriodically();
        gps.getHourMinuteSecond(&hour, &minute, &second);
        display.showNumber((minute * 100) + second, DEC);  
      }
      if (keynum == 18) {
        // B
        char test_string[] = "PRINTER TEST STRING";
        printer.print(test_string);
      }
      if (keynum == 19) {
        // C
        if (sd.status()) {
          display.good();
        } else {
          display.bad();
        }
      }
      if (keynum == 20) {
        int hour, minute, second, millisecond;
        bool res = gps.current_time(micros(), &hour, &minute, &second, &millisecond);
        Serial.print("Res: ");
        Serial.println(res);
        char data[20];
        sprintf(data, "%02d:%02d:%02d:%03d", hour, minute, second, millisecond);
        Serial.println(data);
      }
    }
  }   
  last_key2 = key;
}


//### Mode 3 - Sensor Tuning
//
//- When the Sensor beam is crossed, no noise. When the sensor is not-crossed, beep continuously.
void mode3_loop() {
  if (!sensor.blocked()) {
    buzzer.beep();
  }
}

//### Mode 4 - Race Setup
//
//- If you press A, toggle between 5/F on digit 1
//- If you press B, toggle between b/A/E on digit 2
//- If you press C, toggle between U/d on digit 3
//- If you press D, toggle between 1..9 on digit 4.
//
bool start = true;
uint8_t difficulty = 0; // 0-B, 1-A, 2-E
bool up = true;
uint8_t number = 1;
void mode4_loop() {
  char key = keypad.readChar();
  if (key != NO_KEY) {
    if (key != last_key4) {
      // New Keypress
      int keynum = keypad.intFromChar(key);
      
      switch(keynum) {
      case 17: // A
        start = !start;
        break;
      case 18: // B
        difficulty = (difficulty + 1) % 3;
        break;
      case 19: // C
        up = !up;
        break;
      case 20: // D
        number = (number + 1) % 10;
        break;
      }
    }
  }
  last_key4 = key;
  display.showConfiguration(start, difficulty, up, number);
}

void build_race_filename(char *filename) {
  sprintf(filename, "%s_%s_%s_%d", difficulty == 0 ? "Beginner" : difficulty == 1 ? "Advanced" : "Expert", up ? "Up" : "Down", start ? "Start" : "Finish", number);
}

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

void clear_display() { 
  display.clear();
}
#include <Fsm.h>

// *****************************************************
State mode0(&mode0_run, NULL, NULL);
State mode1(&clear_display, &mode1_loop, NULL);
State mode2(&clear_display, &mode2_loop, NULL);
State mode3(&clear_display, &mode3_loop, NULL);
State mode4(&clear_display, &mode4_loop, NULL);
State mode5(&mode5_setup, &mode5_loop, &mode5_teardown);
State mode6(&mode6_setup, &mode6_loop, &mode6_teardown);

Fsm mode_fsm(&mode0);

#define MODE_OFFSET 100
#define MODE_1 101
#define MODE_2 102
#define MODE_3 103
#define MODE_4 104
#define MODE_5 105
#define MODE_6 106

// MODE Selection FSM
void loop() {
  mode_fsm.run_machine();
  
  gps.readData();
  checkForModeSelection();
}
void setup_fsm() {
  mode_fsm.add_timed_transition(&mode0, &mode1, 1000, NULL); // Go to Mode 1 after 1 second

  // Set up transitions between each possible state and each other state, based on MODE_1, MODE_2, etc triggers.
  State *mode_states[] = { &mode1, &mode2, &mode3, &mode4, &mode5, &mode6};
  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 6; j++) {
      if (j == i) continue; // Don't need to transition from state to same state.
      mode_fsm.add_transition(mode_states[i], mode_states[j], MODE_OFFSET + j + 1, NULL);
    }
  }
}

  
// *****************************************************
// Mode 5 FSM

// Data
int racer_number = 0;
char last_key_pressed = NO_KEY;

// Methods
void clear_racer_number() {
  racer_number = 0;
  display.clear();
}

// Is the racer number already 3 digits long?
// if so, another digit will be "too long"
bool three_digits_racer_number() {
  return racer_number > 99;
}

// Add a new digit to the current racer number
void store_racer_number() {
  Serial.println("Storing Racer number");
  racer_number = (racer_number * 10) + keypad.intFromChar(last_key_pressed);
  Serial.print("Racer #: ");
  Serial.println(racer_number);
  display.showNumber(racer_number);
}

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
  last_key_pressed = keypad.readChar();
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
  last_key_pressed = keypad.readChar();
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
  sprintf(full_string, "RACER %d - %s", racer_number, data_string);
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
void mode5_setup() {
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
  
  last_key_pressed = keypad.readChar();
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

void mode6_setup() {
  mode6_fsm.add_transition(&mode6_initial, &mode6_digits_entered, NUMBER_PRESSED, &store_racer_number);
  mode6_fsm.add_transition(&mode6_initial, &mode6_initial, SENSOR, &store_timing_data);
  
  mode6_fsm.add_transition(&mode6_digits_entered, &mode6_initial, DELETE, &clear_racer_number);
  mode6_fsm.add_transition(&mode6_digits_entered, &mode6_digits_entered, NUMBER_PRESSED, &store_racer_number);
  mode6_fsm.add_transition(&mode6_digits_entered, &mode6_initial, ACCEPT, &mode6_store_result);
 
  sensor.attach_interrupt(); 
}

void mode6_teardown() {
  sensor.detach_interrupt();
}

// When a digit has been entered, monitor for A, D, #
void mode6_digit_check() {
  last_key_pressed = keypad.readChar();
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
    sprintf(full_string, "RACER %d - %s", racer_number, data_string);
    Serial.println(full_string);
    printer.print(full_string);
    sd.writeFile(filename, full_string);
    clear_racer_number();  
  }
}
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
