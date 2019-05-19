#ifndef UNI_DISPLAY_H
#define UNI_DISPLAY_H
#include "Adafruit_LEDBackpack.h"

class UniDisplay
{
  public:
    UniDisplay(int i2c_addr);
    void setup();
    void countdown();
    void show(int, int);
  private:
    int _i2c_addr;
    Adafruit_7segment _display;
};
#endif
