#ifndef UNI_DISPLAY_H
#define UNI_DISPLAY_H
#include "Adafruit_LEDBackpack.h"

class UniDisplay
{
  public:
    UniDisplay(int i2c_addr);
    void setup();
    void all();
    void good();
    void bad();
    void sens();
    void sd();
    void gps();
    void setBlink(bool blink);
    void showConfiguration(bool start, uint8_t difficulty, bool up, uint8_t number);
    void show(char);
    void showNumber(int);
    void showNumber(int, int);
    void showEntriesRemaining(int);
    void clear();
  private:
    int _i2c_addr;
    Adafruit_7segment _display;
};
#endif
