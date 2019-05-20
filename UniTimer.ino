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
//
//
// Needed Libraries
// - Download and provide https://github.com/joeyoung/arduino_keypads/blob/master/Keypad_I2C/Keypad_I2C.h/.cpp in the Keypad_I2C folder
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
//#define ENABLE_GPS
//#define ENABLE_RTC
//#define ENABLE_DISPLAY
//#define ENABLE_KEYPAD
#define ENABLE_PRINTER
//#define ENABLE_SD
//#define ENABLE_SD2
//#define ENABLE_SENSOR
//#define ENABLE_BUZZER

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
#include <SoftwareSerial.h>
#include <Adafruit_Thermal.h>
#endif
// - SD Card
#ifdef ENABLE_SD
#include <SD.h>
#endif
#ifdef ENABLE_SD2
#include <SPI.h>
#include "SdFat.h"
#endif
// - BUZZER
// - BUTTON


/* *************************** (Defining Global Variables) ************************** */
// - SENSOR
#define SENSOR_DIGITAL_INPUT 5
// - GPS
#define GPS_PPS_DIGITAL_INPUT 2
//#define GPSECHO true // for debugging
#define GPS_DIGITAL_OUTPUT 9 // hardware serial #2
#define GPS_DIGITAL_INPUT 10 // hardware serial #2
// - RTC
#define RTC_SQW_DIGITAL_INPUT 2
#define RTC_I2CADDR 0x68
// - DISPLAY
#define DISPLAY_I2CADDR 0x70
// - KEYPAD
#define KEYPAD_COLUMN_WIRE_1 14
#define KEYPAD_COLUMN_WIRE_2 15
#define KEYPAD_COLUMN_WIRE_3 16
#define KEYPAD_COLUMN_WIRE_4 17
#define KEYPAD_ROW_WIRE_1 20
#define KEYPAD_ROW_WIRE_2 21
#define KEYPAD_ROW_WIRE_3 22
#define KEYPAD_ROW_WIRE_4 23
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
//const byte rows = 4; // number of lines
//const byte cols = 4; //Number of columns
//byte linePins[rows] = KEYPAD_ROW_WIRES; // lines pins
//byte columnPins [cols] = KEYPAD_COLUMN_WIRES; // columns pins
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
SoftwareSerial printerSerial(PRINTER_DIGITAL_INPUT, PRINTER_DIGITAL_OUTPUT); // Declare SoftwareSerial obj first
Adafruit_Thermal printer(&printerSerial);     // Pass addr to printer constructor
#endif

// SD
#ifdef ENABLE_SD2
SdFat SD;
File myFile;
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
  #endif
  

  // PRINTER
  #ifdef ENABLE_PRINTER
  printerSerial.begin(19200); // this printer has a 19200 baud
  printer.begin();
  printer.inverseOff();
  printer.println("Hello Robin");
  printer.feed(2);
  printer.sleep();
  // printer.wake();
  #endif

  #ifdef ENABLE_SD2
  Serial.print("Initializing SD card...");

  if (!SD.begin(SD_SPI_CHIP_SELECT_OUTPUT)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  writeFile("testfile.txt", "hEllo Robin");
  writeFile("testfile.txt", "Goodbye Robin");
  readFile("testfile.txt");
  #endif

  #ifdef ENABLE_BUZZER
  pinMode(BUZZER_DIGITAL_OUTPUT, OUTPUT);
  #endif
}

/************************************* (main program) ********* *****************************/


void loop () {
  #ifdef ENABLE_KEYPAD
  keypad.loop();
  #endif
  
  #ifdef ENABLE_SENSOR
//  Serial.print(digitalRead(SENSOR_DIGITAL_INPUT));
  if(digitalRead(SENSOR_DIGITAL_INPUT)) {
    digitalWrite(LED_BUILTIN, LOW);     // turn off led
  } else {
    digitalWrite(LED_BUILTIN, HIGH);    // flash the led
    beep();
  }
  #endif
  
  #ifdef ENABLE_GPS
  gps.loop();
  #endif

  #ifdef ENABLE_RTC
  rtc.loop();
  #endif
  
  
}

/* ********************** Helper Methods ************** */
void beep() {
  Serial.println("Beep");
  #ifdef ENABLE_BUZZER
  tone(BUZZER_DIGITAL_OUTPUT, 1000, 100);
  #endif
}




#ifdef ENABLE_SD2
void writeFile(char *filename, char *text) {
// open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open(filename, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to ");
    Serial.println(filename);
    myFile.println(text);
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.print("error opening ");
    Serial.println(filename);
  }
}

void readFile(char *filename) {
  // re-open the file for reading:
  myFile = SD.open(filename);
  if (myFile) {
    Serial.println(filename);

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.print("error opening ");
    Serial.println(filename);
  }
}
#endif
