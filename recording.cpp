#include "uni_display.h"
#include "uni_keypad.h"
#include "recording.h"

extern UniDisplay display;
extern UniKeypad keypad;

int _racer_number = 0;

// Add a new digit to the current racer number
void store_racer_number() {
  Serial.println("Storing Racer number");
  char last_key_pressed = keypad.lastKeyPressed();
  _racer_number = (_racer_number * 10) + keypad.intFromChar(last_key_pressed);
  Serial.print("Racer #: ");
  Serial.println(_racer_number);
  display.showNumber(_racer_number);
}

// Methods
void clear_racer_number() {
  _racer_number = 0;
  display.clear();
}

int racer_number() {
  return _racer_number;
}

// Is the racer number already 3 digits long?
// if so, another digit will be "too long"
bool three_digits_racer_number() {
  return racer_number() > 99;
}



// **((((((((( NEW FILE ))))))))))))))))) 

Config _config = {true, 0, true, 1};

Config *getConfig() {
  return &_config;
}

void build_race_filename(char *filename) {
  sprintf(filename, "%s_%s_%s_%d", _config.difficulty == 0 ? "Beginner" : _config.difficulty == 1 ? "Advanced" : "Expert", _config.up ? "Up" : "Down", _config.start ? "Start" : "Finish", _config.number);
}
