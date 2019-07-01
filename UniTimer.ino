/* ****************************************************************************************** */
// UniTimer
//
// This application interacts with an optical sensor device, and prints the results
// to a thermal printer as well as displays to a 7-segment display
//
// Expected Hardware Components
// - SENSOR - Sensor
// - GPS - GPS Sensor, for setting accurate time signal
// - RTC - Real Time Clock
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
// [1] The GPS is used to know the absolute time. The RTC clock is used to keep things accurate
//     if we lose GPS lock.
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
//#define ENABLE_RTC
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
#ifdef ENABLE_RTC
#include "uni_rtc.h"
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
// - RTC
#define RTC_SQW_DIGITAL_INPUT 2
#define RTC_I2CADDR 0x68
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

#ifdef ENABLE_RTC
UniRtc rtc(RTC_SQW_DIGITAL_INPUT);
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
#endif

  // GPS
#ifdef ENABLE_GPS
  gps.setup();
#endif

  // RTC
#ifdef ENABLE_RTC
  rtc.setup();
  rtc.setDateTime(2019,6,20,10,54,2);
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
int _mode = 2;
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
//  checkForModeSelection();
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
//      int keynum = keypad.intFromChar(key);
//      if (keynum >= 0 && keynum <= 9) display.show(keynum, DEC); // 0-9
//      if (keynum == 17) display.show(1, DEC); // A
//      if (keynum == 18) display.show(1, DEC); // B
//      if (keynum == 19) display.show(1, DEC); // C
//      if (keynum == 20) display.show(1, DEC); // D
//      if (keynum == 243) { } // #
//      if (keynum == 250) { } // *
//      buzzer.beep();
      display.show(key);
//      Serial.println("Number");
//      Serial.println(keynum);
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
        display.show((minute * 100) + second, DEC);  
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
//- Press C+* If you need to cancel the previosu rider's start time.
//  - This will print and record the cancellation of the previous start time
//
void mode5_loop() {
  // States:
  // INITIAL
  // ONE_DIGIT_ENTERED
  // TWO_DIGITS_ENTERED
  // THREE_DIGITS_ENTERED
  // READY_FOR_SENSOR

  // Transitions
  // INITIAL:
  // - 0-9 -> ONE_DIGIT_ENTERED
  // - C+* -> (Cancel Previous Data AND) INITIAL (Can you register both C + * at the same time?)
  // ONE_DIGIT_ENTERED:
  // - 0-9 -> TWO_DIGITS_ENTERED
  // - A -> READY_FOR_SENSOR
  // - D -> INITIAL
  // TWO_DIGITS_ENTERED:
  // - 0-9 -> THREE_DIGITS_ENTERED
  // - A -> READY_FOR_SENSOR
  // - D -> INITIAL
  // THREE_DIGITS_ENTERED:
  // - 0-9 -> (ERR) INITIAL
  // - A -> READY_FOR_SENSOR
  // - D -> INITIAL
  // READY_FOR_SENSOR:
  // - SENSOR -> (BEEP, DISPLAY AND RECORD AND) INITIAL
  // - A -> READY
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
bool sensor_blocked() {
  return digitalRead(SENSOR_DIGITAL_INPUT);
}

boolean printed = false;
char *time_string = (char *) malloc(25);
void oldloop () {
#ifdef ENABLE_KEYPAD
  keypad.loop();
#endif

#ifdef ENABLE_SENSOR
  //  Serial.print(digitalRead(SENSOR_DIGITAL_INPUT));
  if (digitalRead(SENSOR_DIGITAL_INPUT)) {
    digitalWrite(LED_BUILTIN, LOW);     // turn off led
    //    Serial.println("TRIP");
    if (!printed) {
      beep();
      printed = true;
      int hour, minute, second;
      #ifdef ENABLE_GPS
      gps.getHourMinuteSecond(&hour, &minute, &second);
      display.show((hour * 1000) + (minute * 100) + second, DEC);
      #endif
      sprintf(time_string, "TIME %d:%d:%d", hour, minute, second);
      printer.print(time_string);
    }
  } else {

    digitalWrite(LED_BUILTIN, HIGH);    // flash the led
    printed = false;
  }
#endif

#ifdef ENABLE_GPS
//  gps.readData();
  gps.printPeriodically();
#endif

#ifdef ENABLE_RTC
  rtc.loop();
  rtc.printPeriodically();
#endif

#if defined(ENABLE_KEYPAD)
  //keypad.printKeypress();

  char key = keypad.readChar();
  if (key != NO_KEY) {
#ifdef ENABLE_DISPLAY
    display.show(keypad.intFromChar(key), DEC);
    if (keypad.intFromChar(key) == 243) { // #
       #ifdef ENABLE_PRINTER
         printer.print("Hello World");
       #endif
    }
#endif

#ifdef ENABLE_RTC
    if (keypad.intFromChar(key) == 250) { // *
      DateTime now = rtc.getDateTime();
      Serial.println(now.minute());
      Serial.println(now.second());
      display.show((now.minute() * 100) + now.second(), DEC);
    }
#endif

#ifdef ENABLE_GPS
    if (keypad.intFromChar(key) == 243) { // #
      int hour, minute, second;
      gps.getHourMinuteSecond(&hour, &minute, &second);
      display.show((minute * 100) + second, DEC);
    }
#endif
    beep();
    // #ifdef ENABLE_BUZZER
    // tone(BUZZER_DIGITAL_OUTPUT, 1000, 100);
    // #endif
  }
#endif
}

/* ********************** Helper Methods ************** */
void beep() {
  Serial.println("Beep");
#ifdef ENABLE_BUZZER
  buzzer.beep();
#endif
}
