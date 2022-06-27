// DISPLAY
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
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
#define LETTER_D 0x5E
#define LETTER_E 0x79
#define LETTER_N 0x54
#define LETTER_O 0x5C
#define LETTER_P 0x73
#define LETTER_U 0x3E

uint8_t difficulty_to_letter_code(uint8_t difficulty);

UniDisplay::UniDisplay(int i2c_addr)
{
  _i2c_addr = i2c_addr;
  _display = Adafruit_7segment();
  _wait_state = 0;
}


void UniDisplay::setup() {
  _display.begin(_i2c_addr);
  Serial.println("Display Done init");
}

void UniDisplay::setBlink(bool blink) {
  _display.blinkRate(blink ? 2 : 0);
}

void UniDisplay::all() {
  _display.print(0x8888, HEX);
  _display.writeDisplay();
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

void UniDisplay::showConfiguration(bool start, uint8_t difficulty, bool up, uint8_t number) {
#if 0
  Serial.print("Start: ");
  Serial.println(start);
  Serial.print("Dfificulty: ");
  Serial.println(difficulty_to_letter_code(difficulty));
  Serial.print("Up: ");
  Serial.println(up);
  Serial.print("Number: ");
  Serial.println(number);
#endif
  _display.writeDigitNum(0, start ? 0x5 : 0xf); // S or F
  _display.writeDigitNum(1, difficulty_to_letter_code(difficulty));
  _display.writeDigitRaw(3, up ? LETTER_U : LETTER_D);
  _display.writeDigitNum(4, number);
  _display.writeDisplay();
}

void UniDisplay::sens() {
  _display.writeDigitNum(0, 5);
  _display.writeDigitRaw(1, LETTER_E);
  _display.writeDigitRaw(3, LETTER_N);
  _display.writeDigitNum(4, 5);
  _display.writeDisplay();
}

boolean isDigit(char c) {
  return (c >= '0') && (c <= '9');
}

boolean isLetter(char c) {
  return (c >= 'A') && (c <= 'F');
}


// Print a character in the first position
void UniDisplay::show(char x) {
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
}

void UniDisplay::showNumber(int x) {
  showNumber(x, DEC);
}

void UniDisplay::showNumber(int x, int y = DEC) {
  _display.print(x, y);
  _display.writeDisplay();
}

void UniDisplay::showEntriesRemaining(int x) {
  _display.clear();
  _display.writeDigitRaw(0, LETTER_E);
  if (x >= 10) {
    _display.writeDigitNum(1, x / 10);
    _display.writeDigitNum(3, x % 10);
  } else {
    _display.writeDigitNum(1, x);
  }
  _display.writeDisplay();
}

void UniDisplay::clear() {
  _display.clear();
  _display.writeDisplay();
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
  _display.clear();
  _display.writeDigitRaw(0, 1 << segment);
  _display.writeDisplay();
}

void UniDisplay::waiting(bool center) {
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
