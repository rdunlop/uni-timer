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
#define LETTER_E 0x79
#define LETTER_N 0x54
#define LETTER_O 0x5C
#define LETTER_U 0x3E

UniDisplay::UniDisplay(int i2c_addr)
{
  _i2c_addr = i2c_addr;
  _display = Adafruit_7segment();
}


void UniDisplay::setup() {
  _display.begin(_i2c_addr);
  Serial.println("Display Done init");
}

void UniDisplay::all() {
  _display.print(0x8888, HEX);
  _display.writeDisplay();
}

void UniDisplay::good() {
  _display.writeDigitNum(1, 0x6);
  _display.writeDigitRaw(2, LETTER_O);
  _display.writeDigitRaw(3, LETTER_O);
  _display.writeDigitNum(4, 0xd);
  _display.writeDisplay();
}

void UniDisplay::bad() {
  _display.writeDigitNum(1, 0xb);
  _display.writeDigitNum(2, 0xa);
  _display.writeDigitNum(3, 0xd);
  _display.writeDisplay();
}

void UniDisplay::showConfiguration(bool start, uint8_t difficulty, bool up, uint8_t number) {
  _display.writeDigitRaw(0, start ? 0x5 : 0xf);
//  _display.writeDigitRaw(1, difficulty_to_letter_code(difficulty));
//  _display.writeDigitRaw(2, up ? LETTER_U : 0xd);
//  _dispaly.writeDigitRaw(3, number);
  _display.writeDisplay();
}

void UniDisplay::sens() {
  _display.writeDigitRaw(0, 5);
  _display.writeDigitRaw(1, LETTER_E);
  _display.writeDigitRaw(3, LETTER_N);
  _display.writeDigitRaw(4, 5);
  _display.writeDisplay();
}
void UniDisplay::show(int x, int y = DEC) {
  _display.print(x, y);
  _display.writeDisplay();
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
