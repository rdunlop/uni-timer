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

#ifdef ENABLE_DISPLAY
  display.countdown();
#endif

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
  if (printer.hasPaper()) {
    Serial.println("printer has paper");
  }
#endif

#ifdef ENABLE_BUZZER
  buzzer.setup();
#endif

#ifdef ENABLE_SD
  sd.setup();
#endif
}

/************************************* (main program) ********* *****************************/

boolean printed = false;
char *time_string = (char *) malloc(25);
void loop () {
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
  gps.loop();
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
