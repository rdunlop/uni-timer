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
// - BUTTON

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

/******** ***********************************(set up)*** *************** **********************/
void setup () {
  // Common
  Serial.begin(115200);
  pinMode (LED_BUILTIN, OUTPUT);

  // SENSOR
#ifdef ENABLE_SENSOR
  pinMode(SENSOR_DIGITAL_INPUT, INPUT);
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

  mode0();
}
// Variables
int _mode = 1;
int _new_mode = 1;

// Check systems, and display Good or Bad on the display
void mode0() {
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

bool sensor_blocked() {
  return digitalRead(SENSOR_DIGITAL_INPUT);
}

bool currentTime(char *output) {
  int hour, minute, second, millisecond;
  bool res = gps.current_time(&hour, &minute, &second, &millisecond);
  Serial.print("Res: ");
  Serial.println(res);
  sprintf(output, "%02d:%02d:%02d:%03d", hour, minute, second, millisecond);
  return true;
}

void change_mode(int new_mode) {
  _mode = new_mode;
  if (_mode == 5) {
    mode5_setup();
  }
}
// Check to see if a new mode is selected
void checkForModeSelection() {
  // Only switch to the new mode after all keys are pressed
  if (_new_mode != _mode && !modeKeypad.anyKeyPressed()) {
    Serial.print("new mode: ");
    Serial.println(_new_mode);
    change_mode(_new_mode);
  }
  
  if (modeKeypad.newKeyPressed()) {
    Serial.println("NEW KEY");
    if (modeKeypad.keyPressed('*')) {
      if (modeKeypad.keyPressed('1')) _new_mode = 1;
      if (modeKeypad.keyPressed('2')) _new_mode = 2;
      if (modeKeypad.keyPressed('3')) _new_mode = 3;
      if (modeKeypad.keyPressed('4')) _new_mode = 4;
      if (modeKeypad.keyPressed('5')) _new_mode = 5;
      if (modeKeypad.keyPressed('6')) _new_mode = 6;
    }
  }
}

void loop() {
  switch(_mode) {
    case 1:
      mode1_loop();
      break;
    case 2:
      mode2_loop();
      break;
    case 3:
      mode3_loop();
      break;
     case 4:
      mode4_loop();
      break;
     case 5:
      mode5_loop();
      break;
     case 6:
      mode6_loop();
      break;
  }
  gps.readData();
  checkForModeSelection();
}

//### Mode 1 - Keypad/Sensor Input Test
//
//- If you press a Key, it will Beep for 100ms, and display the number on the display.
//- If you press A, it will display A
//- If you press B, it will display b
//- If you press C, it will display C
//- If you press D, it will display d
//- If you block the Sensor, or un-block the sensor, it will display 5En5 and beep for 100ms
char last_key = NO_KEY;
bool last_sensor = false;
void mode1_loop() {
  //keypad.printKeypress();

  char key = keypad.readChar();
  if (key != NO_KEY) {
    if (key != last_key) {
      // New Keypress
      display.show(key);
      Serial.println("Number");
      Serial.println(key);
    }
  }
  last_key = key;

  bool sensor = sensor_blocked();
  if (last_sensor != sensor) {
    display.sens();
    buzzer.beep();
    last_sensor = sensor;
  }
}

//### Mode 2 - GPS/Printer/SD Test
//
//- If you press A, it will show the GPS time, and beep positively.
//- If you press B, it will show print a test line on the printer.
//- If you press C, it will test writing/reading from the SD card, and display either 6ood or bAd
void mode2_loop() {
  char key = keypad.readChar();
  if (key != NO_KEY) {
    if (key != last_key) {
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
        bool res = gps.current_time(&hour, &minute, &second, &millisecond);
        Serial.print("Res: ");
        Serial.println(res);
        char data[20];
        sprintf(data, "%02d:%02d:%02d:%03d", hour, minute, second, millisecond);
        Serial.println(data);
      }
    }
  }   
  last_key = key;
}


//### Mode 3 - Sensor Tuning
//
//- When the Sensor beam is crossed, no noise. When the sensor is not-crossed, beep continuously.
void mode3_loop() {
  if (!sensor_blocked()) {
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
    if (key != last_key) {
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
  last_key = key;
  display.showConfiguration(start, difficulty, up, number);
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

#include <Fsm.h>

// Data
int racer_number = 0;
char last_key_pressed = NO_KEY;

// Methods
void clear_racer_number() {
  racer_number = 0;
  display.showNumber(racer_number);
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

Fsm fsm(&initial);
#define NUMBER_PRESSED 1
#define DELETE 2
#define ACCEPT 3
#define CANCEL 4
#define SENSOR 5

void initial_check() {
  last_key_pressed = keypad.readChar();
  if (keypad.isDigit(last_key_pressed)) {
    fsm.trigger(NUMBER_PRESSED);
  } else if(sensor_blocked()) {
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
  last_key_pressed = keypad.readChar();
  if (keypad.isDigit(last_key_pressed)) {
    fsm.trigger(NUMBER_PRESSED);
  } else if (keypad.keyPressed('A')) {
    fsm.trigger(ACCEPT);
  } else if (keypad.keyPressed('D')) {
    fsm.trigger(DELETE);
  } else if (sensor_blocked()) {
    buzzer.beep();
    display.sens();
  }
#ifdef FSM_DEBUG
  Serial.println("Digit Check");
#endif
}

void sensor_check() {
  if (keypad.keyPressed('D')) {
    Serial.println("D PRessed");
    fsm.trigger(DELETE);
  } else if (sensor_blocked()) {
    fsm.trigger(SENSOR);
  }
#ifdef FSM_DEBUG
  Serial.println("Sensor Check");
#endif
}

void sensor_triggered() {
  // TODO: Replace this with an interrupt handler?
  buzzer.beep();
  display.sens();
  char racer_string[25];
  char data_string[25];
  currentTime(data_string);
  sprintf(racer_string, "RACER: %d", racer_number);
  Serial.println(racer_string);
  Serial.println(data_string);
  printer.print(racer_string);
  printer.print(data_string);
  clear_racer_number();
}

void sensor_entry() {
  display.setBlink(true);
}

void sensor_exit() {
  display.setBlink(false);
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
  fsm.add_transition(&initial, &one_digit_entered, NUMBER_PRESSED, &store_racer_number);
  
  fsm.add_transition(&one_digit_entered, &initial, DELETE, &clear_racer_number);
  fsm.add_transition(&one_digit_entered, &two_digits_entered, NUMBER_PRESSED, &store_racer_number);
  fsm.add_transition(&one_digit_entered, &ready_for_sensor, ACCEPT, NULL);

  fsm.add_transition(&two_digits_entered, &initial, DELETE, &clear_racer_number);
  fsm.add_transition(&two_digits_entered, &three_digits_entered, NUMBER_PRESSED, &store_racer_number);
  fsm.add_transition(&two_digits_entered, &ready_for_sensor, ACCEPT, NULL);

  fsm.add_transition(&three_digits_entered, &initial, DELETE, &clear_racer_number);
  fsm.add_transition(&three_digits_entered, &initial, NUMBER_PRESSED, &clear_racer_number); // TODO: add better error transition?
  fsm.add_transition(&three_digits_entered, &ready_for_sensor, ACCEPT, NULL);

  fsm.add_transition(&ready_for_sensor, &initial, SENSOR, &sensor_triggered);
  fsm.add_transition(&ready_for_sensor, &initial, DELETE, NULL);
  
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
  fsm.run_machine();
}

//### Mode 6 - Race Run (Finish Line)
//
//- When a sensor is triggered, display E1 to indicate that you need to enter 1 racer number.
//  - It will beep periodically to indicate this
//  - If you have 2 times recorded, it will beep twice periodically, etc.
//- when you press number keys, display the numbers on the display.
//- If you enter more than 3 digits, it will beep and clear
//- If you press "A", it will accept the input, and display the time and the racer number to printer/SD
//- If you press "C", it will clear the display
//- If you press "B", it will duplicate the last time, and create E2
//- If you press D+* it will clear the last entry
void mode6_loop() {
  
}

// ------------------------------------------
