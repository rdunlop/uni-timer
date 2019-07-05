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

  // Use locals to make easy declaration/representation
  char layout[4][4] = {
    { '1', '2', '3', 'A'}, 
    { '4', '5', '6', 'B'},
    { '7', '8', '9', 'C'},
    { '*', '0', '#', 'D'}
  };
  // Copy the locals into the class-level long-term memory element
  for(int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      keyLayout[i][j] = layout[i][j];
    }
  }
}

void UniKeypad::setup() {
  // Here you can enter the symbols of your Keypad
  // NOTE: The keypad library REQUIRES that all of the arrays that are passed in for configuration
  // Are declared in some form of long-term storage (like global static variables, etc).
  // Using 'locals' will NOT work.
  _keypad = new Keypad(makeKeymap (keyLayout), linePins, columnPins, 4, 4); 
  Serial.println("Keypad Done init");
}

void UniKeypad::loop() { }

// How to make this work when there are multiple consumers of this data (the main loop, as well as the currently-selected loop)
bool UniKeypad::newKeyPressed() {
  // Fills kpd.key[ ] array with up-to 10 active keys.
  // Returns true if there are ANY active keys.
  return _keypad->getKeys();
//  for (int i=0; i<LIST_MAX; i++)   // Scan the whole key list.
//  {
//    if (_keypad->key[i].kstate == PRESSED) return true;
//  }
//
//  return false;
}

// return true if the given key is currently pressed or held
bool UniKeypad::keyPressed(char key) {
  // Because the keypad library marks a key as "HOLD"
  // After 500ms, we don't use isPressed
  //  return _keypad->isPressed(key);
  // Instead, we check all of the keys in the list.
  
  // Scan the whole key list.
  for (int i=0; i<LIST_MAX; i++) {
    if (_keypad->key[i].kchar == key) {
      if ((_keypad->key[i].kstate == PRESSED) || (_keypad->key[i].kstate == HOLD)) {
        return true;
      }
    }
  }

  return false;
}

bool UniKeypad::digitPressed() {
//  _keypad->getKeys();
  for (int i = 0; i < 10; i++) {
    if (keyPressed('0' + i)) return true;
  }
  return false;
}

void UniKeypad::printKeypress() {
  char read_key = _keypad->getKey();

  if (read_key != NO_KEY) {
    Serial.println ("read");
    Serial.println(read_key);
    if (isDigit(read_key)) {
      Serial.print("value: ");
      Serial.println(intFromChar(read_key));
    } else {
//      beep();
    }
  }
}

char UniKeypad::readChar() {
  return _keypad->getKey();
}

// Return a digit if it is pressed
// return -1 if no digit is pressed
uint8_t UniKeypad::readDigit() {
  _keypad->getKeys();
  for (int i = 0; i < 10; i++) {
    if (keyPressed('0' + i)) return i;
  }
  return -1;
}

boolean UniKeypad::isDigit(char c) {
  return (c >= '0') && (c <= '9');
}

uint8_t UniKeypad::intFromChar(char c) {
  return c - '0';
}
