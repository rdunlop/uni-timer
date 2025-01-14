#include "uni_keypad.h"
#include "uni_display.h"
#include "uni_config.h"
#include "modes.h"

#include "recording.h"

extern UniKeypad keypad;
extern UniDisplay display;
extern UniConfig config;

void displayFileConfig();
void filename_config(char key);
void racer_digits_config(char key);
void start_line_config(char key);
void finish_line_config(char key);


//### Mode 4.1 - Race Setup
//
//- If you press A, toggle between 5/F on digit 1
//- If you press B, toggle between b/A/E on digit 2
//- If you press C, toggle between U/d on digit 3
//- If you press D, toggle between 1..9 on digit 4.
//
char last_key4 = NO_KEY;

void mode4_setup() {
}

int config_mode = 1;

void mode4_loop() {
  char key = keypad.readChar();
  if (key != NO_KEY) {
    if (key != last_key4) {
      // New Keypress
      switch(key) {
        case '1':
          config_mode = 1;
          break;
        case '2':
          config_mode = 2;
          break;
        case '3':
          config_mode = 3;
          break;
        case '4':
          config_mode = 4;
          break;
      }
      switch(config_mode) {
        case 1:
          filename_config(key);
          break;
        case 2:
          racer_digits_config(key);
          break;
        case 3:
          start_line_config(key);
          break;
        case 4:
          finish_line_config(key);
          break;
      }
    }
  }
  last_key4 = key;
  
}

void mode4_teardown() {
  config.writeConfig();
}

void racer_digits_config(char key) {
  switch(key) {
    case 'A':
      config.toggle_bib_number_length();
      break;
  }
  display.showNumber(config.get_bib_number_length());
}

void start_line_config(char key) {
  switch(key) {
    case 'A':
      config.toggle_start_line_countdown();
      break;
  }
  display.showNumber(config.get_start_line_countdown() ? 1 : 2);
}

void finish_line_config(char key) {
  switch(key) {
    case 'A':
      config.reset_finish_line_spacing();
      break;
    case 'B':
      config.increment_finish_line_spacing(10);
      break;
    case 'C':
      config.increment_finish_line_spacing(100);
      break;
  }
  display.showNumber(config.get_finish_line_spacing());
}

void filename_config(char key) {
  switch(key) {
  case 'A':
    config.toggle_start();
    break;
  case 'B':
    config.increase_difficulty();
    break;
  case 'C':
    config.toggle_up();
    break;
  case 'D':
    config.increment_race_number();
    break;
  }
  displayFileConfig();
}

void displayFileConfig() {
  display.showConfiguration(config.get_start(), config.get_difficulty(), config.get_up(), config.get_race_number());
}
