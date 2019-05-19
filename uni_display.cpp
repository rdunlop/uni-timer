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
  _display.print(0x8888, HEX);
  _display.writeDisplay();
}

void UniDisplay::countdown() {
  for (uint16_t counter = 5; counter > 0; counter--) {
    _display.println(counter);
    _display.writeDisplay();
    delay(1000);
  }
}

void UniDisplay::show(int x, int y = DEC) {
  _display.print(x, y);
  _display.writeDisplay();
}
