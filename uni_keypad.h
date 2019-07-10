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
    bool newKeyPressed();
    bool keyPressed(char key);
    bool anyKeyPressed();
    bool digitPressed();
    char readChar();
    uint8_t readDigit();
    uint8_t intFromChar(char);
    boolean isDigit(char);
    char lastKeyPressed();
  private:
    byte linePins[4];
    byte columnPins[4];
    char keyLayout [4][4];
    Keypad *_keypad;
    char _last_key_pressed;
};

#endif
