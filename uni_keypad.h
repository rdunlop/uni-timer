#ifndef UNI_KEYPAD_H
#define UNI_KEYPAD_H
#include "Keypad.h"

class UniKeypad
{
  public:
    UniKeypad(byte,byte,byte,byte, byte,byte,byte,byte);
    void setup();
    void loop();
  private:
    byte linePins[4];
    byte columnPins[4];
    Keypad _keypad;
    boolean isDigit(char);
    uint8_t intFromChar(char);
};

#endif
