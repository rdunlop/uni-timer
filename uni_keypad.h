#ifndef UNI_KEYPAD_H
#define UNI_KEYPAD_H
#include "Keypad.h"

class UniKeypad
{
  public:
    UniKeypad(byte,byte,byte,byte, byte,byte,byte,byte);
    void setup();
    void loop();
    void printKeypress();
    char readChar();
    uint8_t intFromChar(char);
    boolean isDigit(char);
  private:
    byte linePins[4];
    byte columnPins[4];
    char keyLayout [4][4];
    Keypad *_keypad;
};

#endif
