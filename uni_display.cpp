// DISPLAY
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include "uni_display.h"

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
  _display.writeDigitRaw(1, 0x6);
  _display.writeDigitRaw(2, 0x0);
  _display.writeDigitRaw(3, 0x0);
  _display.writeDigitRaw(4, 0xd);
  _display.writeDisplay();
}

void UniDisplay::bad() {
  _display.writeDigitRaw(1, 0xb);
  _display.writeDigitRaw(2, 0xa);
  _display.writeDigitRaw(3, 0xd);
  _display.writeDisplay();
}

void UniDisplay::show(int x, int y = DEC) {
  _display.print(x, y);
  _display.writeDisplay();
}
