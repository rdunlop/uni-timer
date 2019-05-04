/* ****************************************************************************************** */
// UniTimer
//
// This application interacts with an optical sensor device, and prints the results
// to a thermal printer as well as displays to a 7-segment display
//
// Expected Hardware Components
// - SENSOR - Sensor
// - RTC - Real Time Clock
// - DISPLAY - 7 Segment display
// - KEYPAD_EXPANSION - I2C expansion board, with keypad connected to it
// - BUZZER - Piezo buzzer
// - BUTTON - Input button
//
// Expected Hardware Setup
// - Keypad is connected to the I2C expansion board, with the leftmost pin of the keypad connected to P0 of the expansion board.
//
// Needed Libraries
// - Download and provide https://github.com/joeyoung/arduino_keypads/blob/master/Keypad_I2C/Keypad_I2C.h/.cpp in the Keypad_I2C folder
// -
//
// NOTES:
// [1] The GPS is used to know the absolute time. The RTC clock is used to keep things accurate
//     if we lose GPS lock.
//     Based on https://wyolum.com/syncing-arduino-with-gps-time/.
//     Whenever we have GPS lock, we keep track of the offset from micros() for the GPS time
//     and we use that offset whenever we are printing the time.
/* ****************************************************************************************** */

// https://www.bastelgarage.ch/index.php?route=extension/d_blog_module/post&post_id=8
// https://github.com/joeyoung/arduino_keypads/blob/master/Keypad_I2C/Keypad_I2C.h


/* *********************** Includes *********************************** */
// - Common
#include "Wire.h"

// - SENSOR
// - GPS
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
// - RTC
#include "RTClib.h"
// - DISPLAY
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
// - KEYPAD_EXPANSION
#include "Keypad.h"
#include "Keypad_I2C.h"
// - PRINTER
#include <Adafruit_Thermal.h>
// - BUZZER
// - BUTTON


/* *************************** (Defining Global Variables) ************************** */
// - SENSOR
#define SENSOR_DIGITAL_INPUT 6 // still not determined
// - GPS
#define GPS_PPS_DIGITAL_INPUT 2 // has to be pin 2 or 3 because of interrupt behavior
#define GPSECHO true // for debugging
#define GPS_DIGITAL_OUTPUT 8 // software serial
#define GPS_DIGITAL_INPUT 7 // software serial
// - RTC
#define RTC_SQW_DIGITAL_INPUT 3 // has to be pin 2 or 3 because of interrupt behavior
#define RTC_I2CADDR 0x00 // UNKNOWN
// - DISPLAY
#define DISPLAY_I2CADDR 0x70
// - KEYPAD_EXPANSION
#define KEYPAD_EXPANSION_I2CADDR 0x38 // I2C address from PCF8574
// - PRINTER
#define PRINTER_DIGITAL_OUTPUT 4 // Arduino transmit  YELLOW WIRE  labeled RX on printer
#define PRINTER_DIGITAL_INPUT 5 // Arduino receive   GREEN WIRE   labeled TX on printer
// - BUZZER
#define BUZZER_DIGITAL_OUTPUT 13
// - BUTTON
#define BUTTON_DIGITAL_INPUT 3 // unused


/* ************************** Initialization ******************* */
// DISPLAY -------------------------------------------
Adafruit_7segment display = Adafruit_7segment();

// KEYPAD --------------------------------------------
const byte rows = 4; // number of lines
const byte cols = 3; //Number of columns

// Here you can enter the symbols of your Keypad
char keyLayout [rows] [cols] = {
  { '1', '2', '3'}, 
  { '4', '5', '6'},
  { '7', '8', '9'},
  { '*', '0', '#'}
};

// Here define how the keypad is wired to the IO pins of the PCF8574.
byte linePins[rows] = {0, 1, 2, 3}; // lines pins
byte columnsPins [cols] = {4, 5, 6}; // columns pins

Keypad_I2C i2cKeypad (makeKeymap (keyLayout), linePins, columnsPins, rows, cols, KEYPAD_EXPANSION_I2CADDR); 

// GPS ------------------------------------------
SoftwareSerial gpsSerial(GPS_DIGITAL_OUTPUT, GPS_DIGITAL_INPUT);
Adafruit_GPS GPS(&gpsSerial);

volatile unsigned long count = 0;
volatile unsigned long rtc_start_ms = micros();
volatile unsigned long pps_start_ms = micros();

// NOTE: The GPS PPS signal will ONLY fire when there is GPS lock.
void pps_interrupt(){
  unsigned long now = micros();
  Serial.print("GPS PPS: ");
  Serial.println(now - pps_start_ms);
  pps_start_ms = now;
}

// RTC -----------------------------------------
RTC_DS3231 rtc;

volatile byte rtc_interrupt_flag = false;

// RTC SQW signal is an active-low signal, so it needs INPUT_PULLUP
void rtc_interrupt(){
  rtc_interrupt_flag = true;
  Serial.print("RTC: ");
  unsigned long  now = micros();
  Serial.println(now - rtc_start_ms);
  rtc_start_ms = now;
}

// PRINTER -------------------------------------
SoftwareSerial printerSerial(PRINTER_DIGITAL_INPUT, PRINTER_DIGITAL_OUTPUT); // Declare SoftwareSerial obj first
Adafruit_Thermal printer(&printerSerial);     // Pass addr to printer constructor

/******** ***********************************(set up)*** *************** **********************/
void setup () {
  // Common
  Serial.begin(115200);
  pinMode (LED_BUILTIN, OUTPUT);

  // DISPLAY
  display.begin(DISPLAY_I2CADDR);
  display.print(0x41, HEX);
  display.writeDisplay();
  delay(500);
  for (uint16_t counter = 5; counter > 0; counter--) {
    display.println(counter);
    display.writeDisplay();
    delay(1000);
  }

  // KEYPAD
  i2cKeypad.begin();        
  

  // GPS
  pinMode(GPS_PPS_DIGITAL_INPUT, INPUT);
  attachInterrupt(digitalPinToInterrupt(GPS_PPS_DIGITAL_INPUT), pps_interrupt, RISING);

  // RTC
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  // I don't know if this does anything
  // enable the 1 Hz output
  //rtc.writeSqwPinMode (DS3231_SquareWave1Hz);

  pinMode(RTC_SQW_DIGITAL_INPUT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RTC_SQW_DIGITAL_INPUT), rtc_interrupt, RISING);

  // PRINTER
  printerSerial.begin(19200); // this printer has a 19200 baud
  printer.begin();        // Init printer (same regardless of serial type)
  printer.inverseOff();
  printer.println("Hello Robin");
  printer.feed(2);
}

/************************************* (main program) ********* *****************************/
uint32_t last_gps_print_time = millis();
uint32_t last_rtc_print_time = millis();
void loop () {
  char read_key = i2cKeypad.getKey ();

  if (read_key != NO_KEY) {
    Serial.println ("read");
    Serial.println(read_key);
    if (isDigit(read_key)) {
      Serial.print("value: ");
      Serial.println(intFromChar(read_key));
      display.print(intFromChar(read_key), DEC);
      display.writeDisplay();
    } else {
      beep();
    }
  }
  Serial.println(digitalRead(SENSOR_DIGITAL_INPUT));
  delay(100);
  if(digitalRead(SENSOR_DIGITAL_INPUT)) {
    digitalWrite(LED_BUILTIN, HIGH);    // flash the led
  } else {
    digitalWrite(LED_BUILTIN, LOW);     // turn off led
  }
//  if (rtc_interrupt_flag) {
//    digitalWrite(LED_BUILTIN, HIGH);    // flash the led
//    delay(100);                         // wait a little bit
//    digitalWrite(LED_BUILTIN, LOW);     // turn off led
//    rtc_interrupt_flag =  false;                      // clear the flag until timer sets it again
//  }

  
  // if millis() or timer wraps around, we'll just reset it
  if (last_gps_print_time > millis())  last_gps_print_time = millis();
  // approximately every 2 seconds or so, print out the current GPS stats
  if (millis() - last_gps_print_time > 2000) { 
    last_gps_print_time = millis(); // reset the timer
    printGPS();
  }

  // if millis() or timer wraps around, we'll just reset it
  if (last_rtc_print_time > millis())  last_rtc_print_time = millis();
  // approximately every 2 seconds or so, print out the current GPS stats
  if (millis() - last_rtc_print_time > 2000) { 
    last_rtc_print_time = millis(); // reset the timer
    printRTC();
  }
}

/* ********************** Helper Methods ************** */
boolean isDigit(char c) {
  return (c >= '0') && (c <= '9');
}

uint8_t intFromChar(char c) {
  return c - '0';
}

void beep() {
  Serial.println("Beep");
}

void printGPS() {
  // read data from the GPS in the 'main loop'
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
  if (GPSECHO)
    if (c) Serial.print(c);
  
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences! 
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    //Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false
  
    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
  }

  Serial.println("---------------------------");
  Serial.println("GPS:");
  Serial.print("\nTime: ");
  Serial.print(GPS.hour, DEC); Serial.print(':');
  Serial.print(GPS.minute, DEC); Serial.print(':');
  Serial.print(GPS.seconds, DEC); Serial.print('.');
  Serial.println(GPS.milliseconds);
  Serial.print("Date: ");
  Serial.print(GPS.day, DEC); Serial.print('/');
  Serial.print(GPS.month, DEC); Serial.print("/20");
  Serial.println(GPS.year, DEC);
  Serial.print("Fix: "); Serial.print((int)GPS.fix);
  Serial.print(" quality: "); Serial.println((int)GPS.fixquality); 
  if (GPS.fix) {
    Serial.print("Location: ");
    Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
    Serial.print(", "); 
    Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
    Serial.print("Location (in degrees, works with Google Maps): ");
    Serial.print(GPS.latitudeDegrees, 4);
    Serial.print(", "); 
    Serial.println(GPS.longitudeDegrees, 4);
    
    Serial.print("Speed (knots): "); Serial.println(GPS.speed);
    Serial.print("Angle: "); Serial.println(GPS.angle);
    Serial.print("Altitude: "); Serial.println(GPS.altitude);
    Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
  }
}

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
void printRTC() {
  DateTime now = rtc.now();

  Serial.println("---------------------------");
  Serial.println("RTC:");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
  
  Serial.print(" since midnight 1/1/1970 = ");
  Serial.print(now.unixtime());
  Serial.print("s = ");
  Serial.print(now.unixtime() / 86400L);
  Serial.println("d");
  
  // calculate a date which is 7 days and 30 seconds into the future
  DateTime future (now + TimeSpan(7,12,30,6));
  
  Serial.print(" now + 7d + 30s: ");
  Serial.print(future.year(), DEC);
  Serial.print('/');
  Serial.print(future.month(), DEC);
  Serial.print('/');
  Serial.print(future.day(), DEC);
  Serial.print(' ');
  Serial.print(future.hour(), DEC);
  Serial.print(':');
  Serial.print(future.minute(), DEC);
  Serial.print(':');
  Serial.print(future.second(), DEC);
  Serial.println();
  
  Serial.println();
}
