#include "uni_keypad.h"
#include "uni_display.h"
#include "modes.h"

#include "recording.h"

extern UniKeypad keypad;
extern UniDisplay display;

//### Mode 4 - Race Setup
//
//- If you press A, toggle between 5/F on digit 1
//- If you press B, toggle between b/A/E on digit 2
//- If you press C, toggle between U/d on digit 3
//- If you press D, toggle between 1..9 on digit 4.
//
char last_key4 = NO_KEY;
void mode4_loop() {
  char key = keypad.readChar();
  if (key != NO_KEY) {
    if (key != last_key4) {
      // New Keypress
      int keynum = keypad.intFromChar(key);
      Config *config = getConfig();
      
      switch(keynum) {
      case 17: // A
        config->start = !config->start;
        break;
      case 18: // B
        config->difficulty = (config->difficulty + 1) % 3;
        break;
      case 19: // C
        config->up = !config->up;
        break;
      case 20: // D
        config->number = (config->number + 1) % 10;
        break;
      }
      last_key4 = key;
      display.showConfiguration(config->start, config->difficulty, config->up, config->number);
    }
  }
  
}
