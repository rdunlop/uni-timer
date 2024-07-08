// DISPLAY
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_LiquidCrystal.h"
#include "uni_display.h"

// 7-Segment codes for displaying some letters.
// Right-most Octet:
// 0x1 - TOP
// 0x2 - RIGHT-top
// 0x4 - RIGHT-bottom
// 0x8 - BOTTOM

// Second Octet:
// 0x1 - LEFT-bottom
// 0x2 - LEFT-top
// 0x4 - CENTER dash
// 0x8 - dot
#define LETTER_A 0x77
#define LETTER_D 0x5E
#define LETTER_E 0x79
#define LETTER_N 0x54
#define LETTER_O 0x5C
#define LETTER_P 0x73
#define LETTER_R 0x50
#define LETTER_U 0x3E

#define DISPLAY_MAX_LINE_LENGTH 16

uint8_t difficulty_to_letter_code(uint8_t difficulty);

// If defined, use the 7-segment display, otherwise ignore this, and move faster
// #define SEVEN_SEGMENT_DISPLAY true

// ***************************
// API
// ***************************
void UniDisplay::setup() {
#ifdef SEVEN_SEGMENT_DISPLAY
  _display.begin(_i2c_addr);
  Serial.println("Display Done init");
#endif

  if (!_lcd.begin(16, 2)) { // should update these pins to be constants
    Serial.println("Could not init backpack. Check wiring.");
  }
//   Print a message to the LCD.
  _lcd.print("UDA Initializing!");
  _lcd.setBacklight(1);
   // set the display to automatically scroll:
  // _lcd.autoscroll();
}

void UniDisplay::sdGood(bool delay_for_segment) {
#ifdef SEVEN_SEGMENT_DISPLAY
  if (delay_for_segment) {
    sd();
    delay(1000);
    good();
  }
#endif
  _lcd.clear();
  _lcd.home();
  _lcd.print("SD Good");
}

void UniDisplay::sdBad(bool delay_for_segment) {
#ifdef SEVEN_SEGMENT_DISPLAY
  if (delay_for_segment) {
    sd();
    delay(1000);
    bad();
  }
#endif
  _lcd.clear();
  _lcd.home();
  _lcd.print("SD Bad");
}

void UniDisplay::radioGood(bool delay_for_segment) {
#ifdef SEVEN_SEGMENT_DISPLAY
  if (delay_for_segment) {
    radio();
    delay(1000);
    good();
  }
#endif
  _lcd.clear();
  _lcd.home();
  _lcd.print("Radio Good");
}

void UniDisplay::radioBad(bool delay_for_segment) {
#ifdef SEVEN_SEGMENT_DISPLAY
  if (delay_for_segment) {
    radio();
    delay(1000);
    bad();
  }
#endif
  _lcd.clear();
  _lcd.home();
  _lcd.print("Radio Bad");
}

void UniDisplay::gpsGood(bool delay_for_segment) {
#ifdef SEVEN_SEGMENT_DISPLAY
  if (delay_for_segment) {
    gps();
    delay(1000);
    good();
  }
#endif
  _lcd.clear();
  _lcd.home();
  _lcd.print("GPS Good");
}

void UniDisplay::gpsBad(bool delay_for_segment) {
#ifdef SEVEN_SEGMENT_DISPLAY
  if (delay_for_segment) {
    gps();
    delay(1000);
    bad();
  }
#endif
  _lcd.clear();
  _lcd.home();
  _lcd.print("GPS Bad");
}

void UniDisplay::allGood() {
#ifdef SEVEN_SEGMENT_DISPLAY
  good();
#endif
  _lcd.clear();
  _lcd.home();
  _lcd.print("All Good");
}

void UniDisplay::notAllGood() {
#ifdef SEVEN_SEGMENT_DISPLAY
  bad();
#endif
  _lcd.clear();
  _lcd.home();
  _lcd.print("NOT All Good");
}

void UniDisplay::displayTest() {
  all();
}
// **************************
// INTERNAL
// **************************
void UniDisplay::setBlink(bool blink) {
  _display.blinkRate(blink ? 2 : 0);
}

void UniDisplay::all() {
#ifdef SEVEN_SEGMENT_DISPLAY
  _display.print(0x8888, HEX);
  _display.writeDisplay();
#endif
  _lcd.clear();
  _lcd.home();
  _lcd.print("1234567890123456");
  _lcd.setCursor(0, 1); // Move to the beginning of the second row
  _lcd.print("1234567890123456");
}

void UniDisplay::good() {
  _display.writeDigitNum(0, 0x6);
  _display.writeDigitRaw(1, LETTER_O);
  _display.writeDigitRaw(3, LETTER_O);
  _display.writeDigitNum(4, 0xd);
  _display.writeDisplay();
}

void UniDisplay::bad() {
  _display.writeDigitRaw(0, 0x0);
  _display.writeDigitNum(1, 0xb);
  _display.writeDigitNum(3, 0xa);
  _display.writeDigitNum(4, 0xd);
  _display.writeDisplay();
}

void UniDisplay::sd() {
  _display.clear();
  _display.writeDigitNum(3, 0x5);
  _display.writeDigitNum(4, 0xd);
  _display.writeDisplay();
}

void UniDisplay::gps() {
  _display.clear();
  _display.writeDigitNum(1, 0x9);
  _display.writeDigitRaw(3, LETTER_P);
  _display.writeDigitNum(4, 0x5);
  _display.writeDisplay();
}

void UniDisplay::radio() {
  _display.clear();
  _display.writeDigitRaw(1, LETTER_R);
  _display.writeDigitRaw(3, LETTER_A);
  _display.writeDigitRaw(4, LETTER_D);
  _display.writeDisplay();
}

void UniDisplay::sens() {
#ifdef SEVEN_SEGMENT_DISPLAY
  _display.writeDigitNum(0, 5);
  _display.writeDigitRaw(1, LETTER_E);
  _display.writeDigitRaw(3, LETTER_N);
  _display.writeDigitNum(4, 5);
  _display.writeDisplay();
#endif

  _lcd.clear();
  _lcd.home();
  _lcd.print("Sensor!");
}

// Indicate that we have the racer number entered, and we're waiting
// for the sensor to be triggered
void UniDisplay::waitingForSensor() {
#ifdef SEVEN_SEGMENT_DISPLAY
  setBlink(true);
#endif
  _lcd.print(" READY!");
  _lcd.blink();
}

void UniDisplay::doneWaitingForSensor() {
#ifdef SEVEN_SEGMENT_DISPLAY
  setBlink(false);
#endif
  _lcd.clear();
  _lcd.noBlink();
}

boolean isDigit(char c) {
  return (c >= '0') && (c <= '9');
}

boolean isLetter(char c) {
  return (c >= 'A') && (c <= 'F');
}


// Print a character in the first position
// Used for testing the keypad
void UniDisplay::show(char x) {
#ifdef SEVEN_SEGMENT_DISPLAY
  if (isDigit(x)) {
    _display.print(x - '0', DEC);
  } else if (isLetter(x)) {
    _display.writeDigitNum(4, (x - 'A') + 0xA);
  } else {
    // is special
    switch(x) {
      case '*':
        _display.writeDigitRaw(4, LETTER_E);
        break;
      case '#':
        _display.writeDigitRaw(4, LETTER_N);
        break;
    }  
  }
  _display.writeDisplay();
#endif

  _lcd.clear();
  _lcd.home();
  _lcd.print(x);
}

void UniDisplay::print(const char *message, const char *message2) {
  // always ensure that we aren't exceeding 16 characters
  char msg1[16 + 1] = {0}; // 1 extra for the final null, if we have 16 char input
  char msg2[16 + 1] = {0}; // 1 extra for the final null, if we have 16 char input
  memcpy(msg1, message, 16);
  memcpy(msg2, message2, 16);
  _lcd.clear();
  _lcd.home();
  _lcd.print(msg1);
  _lcd.setCursor(0, 1); // Move to the beginning of the second row
  _lcd.print(msg2);
}

void UniDisplay::showTimeResult(TimeResult *time_result) {
#ifdef SEVEN_SEGMENT_DISPLAY
  showNumber(time_result->minute);
  delay(500);
  showNumber(time_result->second);
  delay(500);
  showNumber(time_result->millisecond);
  delay(500);
#endif
  _lcd.clear();
  _lcd.home();

  // Extract into common helper?
  char data_string[DISPLAY_MAX_LINE_LENGTH];
  snprintf(data_string, DISPLAY_MAX_LINE_LENGTH, "%02d:%02d.%03d", time_result->minute, time_result->second, time_result->millisecond);
  _lcd.print(data_string);
}

void UniDisplay::showNumber(int x) {
#ifdef SEVEN_SEGMENT_DISPLAY
  _display.print(x, DEC);
  _display.writeDisplay();
#endif

  _lcd.clear();
  _lcd.home();
  _lcd.print(x);
}

void UniDisplay::displayConfig(UniConfig *config) {
#ifdef SEVEN_SEGMENT_DISPLAY
  _display.writeDigitNum(0, config->get_start() ? 0x5 : 0xf); // S or F
  _display.writeDigitNum(1, difficulty_to_letter_code(config->get_difficulty()));
  _display.writeDigitRaw(3, config->get_up() ? LETTER_U : LETTER_D);
  _display.writeDigitNum(4, config->get_race_number());
  _display.writeDisplay();
#endif
  _lcd.clear();
  _lcd.home();
  _lcd.print(config->get_start() ? "Start" : "Finish"); // 6
  _lcd.print("-"); // 7
  switch(config->get_difficulty()) {
    case 0: // Beginner
      _lcd.print("Beginner"); // 15
      break;
     case 1: // Advanced
      _lcd.print("Advanced"); // 15
      break;
     case 2: // Expert
      _lcd.print("Expert"); // 13
      break;
  }
  _lcd.setCursor(0, 1);
  _lcd.print(config->get_up() ? "up" : "down"); // 4
  _lcd.print(" - Race:"); // 12
  _lcd.print(config->get_race_number()); // 13
}

void UniDisplay::showRacerDigits(int numDigits) {
#ifdef SEVEN_SEGMENT_DISPLAY
  showNumber(numDigits);
#endif

  _lcd.clear();
  _lcd.home();
  _lcd.print("Racer Bib Digits:");
  _lcd.setCursor(0, 1);
  _lcd.print(numDigits);
}
void UniDisplay::startLineCountdown(bool startLineMode) {
#ifdef SEVEN_SEGMENT_DISPLAY
  showNumber(startLineMode ? 1 : 2);
#endif

  _lcd.clear();
  _lcd.home();
  _lcd.print("Start Line Mode:");
  _lcd.setCursor(0, 1);
  if (startLineMode) {
    _lcd.print("ENABLED");
  } else {
    _lcd.print("Disabled");
  }
}
void UniDisplay::triggerIntervalDelay(uint16_t interval) {
#ifdef SEVEN_SEGMENT_DISPLAY
  showNumber(interval);
#endif

  _lcd.clear();
  _lcd.home();
  _lcd.print("Sensor Trigger");
  _lcd.setCursor(0, 1);
  _lcd.print("Interval: ");
  _lcd.print(interval);
  _lcd.print("ms");
}


void UniDisplay::showEntriesRemaining(int x) {
#ifdef SEVEN_SEGMENT_DISPLAY
  _display.clear();
  _display.writeDigitRaw(0, LETTER_E);
  if (x >= 10) {
    _display.writeDigitNum(1, x / 10);
    _display.writeDigitNum(3, x % 10);
  } else {
    _display.writeDigitNum(1, x);
  }
  _display.writeDisplay();
#endif
  _lcd.clear();
  _lcd.home();
  _lcd.print("Entries:");
  _lcd.setCursor(0, 1);
  _lcd.print(x);
}

void UniDisplay::clear() {
#ifdef SEVEN_SEGMENT_DISPLAY
  _display.clear();
  _display.writeDisplay();
#endif
  _lcd.clear();
  _lcd.home();
}

// create a wait-indicator movement
// ----------------------------- move elsewhere?
#include <Fsm.h>

extern UniDisplay display;
void display0() { display.showWaiting(false, 0); }
void display1() { display.showWaiting(false, 1); }
void display2() { display.showWaiting(false, 2); }
void display3() { display.showWaiting(false, 3); }
void display4() { display.showWaiting(false, 4); }
void display5() { display.showWaiting(false, 5); }
State segment0(&display0, NULL, NULL);
State segment1(&display1, NULL, NULL);
State segment2(&display2, NULL, NULL);
State segment3(&display3, NULL, NULL);
State segment4(&display4, NULL, NULL);
State segment5(&display5, NULL, NULL);

Fsm waiting_fsm(&segment0);


// Display a moving indicator, around the circle of digit 1
void UniDisplay::showWaiting(bool center, int segment) {
#ifdef SEVEN_SEGMENT_DISPLAY
  _display.clear();
  _display.writeDigitRaw(0, 1 << segment);
  _display.writeDisplay();
#endif
}

void UniDisplay::waitingPattern() {
#ifdef SEVEN_SEGMENT_DISPLAY
  if (_wait_state == 0) {
    waiting_fsm.add_timed_transition(&segment0, &segment1, 1000, NULL);
    waiting_fsm.add_timed_transition(&segment1, &segment2, 1000, NULL);
    waiting_fsm.add_timed_transition(&segment2, &segment3, 1000, NULL);
    waiting_fsm.add_timed_transition(&segment3, &segment4, 1000, NULL);
    waiting_fsm.add_timed_transition(&segment4, &segment5, 1000, NULL);
    waiting_fsm.add_timed_transition(&segment5, &segment0, 1000, NULL);
    _wait_state = 1;
  }
  waiting_fsm.run_machine();
#endif
}

void UniDisplay::waitingForGps() {
  _lcd.clear();
  _lcd.home();
  _lcd.print("Waiting for GPS");
  _lcd.blink();
}

void UniDisplay::doneWaitingForGps() {
  _lcd.noBlink();
  _lcd.clear();
}

void UniDisplay::configNotFound() {
  _lcd.clear();
  _lcd.home();
  _lcd.print("Config Not Found");
}

void UniDisplay::configLoaded() {
  _lcd.clear();
  _lcd.home();
  _lcd.print("Config Loaded!");
}

uint8_t difficulty_to_letter_code(uint8_t difficulty) {
  switch(difficulty) {
    case 0: // Beginner
      return 0xb;
      break;
     case 1: // Advanced
      return 0xA;
      break;
     case 2: // Expert
      return 0xE;
      break;
  }
  return 0x0;
}
