// - KEYPAD
#include "uni_keypad.h"

UniKeypad::UniKeypad(byte r1, byte r2, byte r3, byte r4, byte c1, byte c2, byte c3, byte c4)
{
  linePins[0] = r1;
  linePins[1] = r2;
  linePins[2] = r3;
  linePins[3] = r4; // lines pins
  columnPins[0] = c1;
  columnPins[1] = c2;
  columnPins[2] = c3;
  columnPins[3] = c4; // columns pins
}

void UniKeypad::setup() {
  // Here you can enter the symbols of your Keypad
  char keyLayout [4] [4] = {
    { '1', '2', '3', 'A'}, 
    { '4', '5', '6', 'B'},
    { '7', '8', '9', 'C'},
    { '*', '0', '#', 'D'}
  };
  _keypad = Keypad(makeKeymap (keyLayout), linePins, columnPins, 4, 4); 
}

void UniKeypad::loop() {
  char read_key = _keypad->getKey ();

  if (read_key != NO_KEY) {
    Serial.println ("read");
    Serial.println(read_key);
    if (isDigit(read_key)) {
      Serial.print("value: ");
      Serial.println(intFromChar(read_key));
      #ifdef ENABLE_DISPLAY
//      display.update(intFromChar(read_key), DEC);
      #endif
      #ifdef ENABLE_BUZZER
      tone(BUZZER_DIGITAL_OUTPUT, 1000, 100);
      #endif
    } else {
//      beep();
    }
  }
}

boolean UniKeypad::isDigit(char c) {
  return (c >= '0') && (c <= '9');
}

uint8_t UniKeypad::intFromChar(char c) {
  return c - '0';
}
