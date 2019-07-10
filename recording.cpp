#include "uni_display.h"
#include "uni_keypad.h"
#include "uni_printer.h"
#include "uni_sd.h"
#include "recording.h"

extern UniDisplay display;
extern UniKeypad keypad;
extern UniPrinter printer;
extern UniSd sd;

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

void print_racer_data_to_printer(int racer_number, TimeResult data) {
  char full_string[25];
  char data_string[25];
  sprintf(data_string, "%02d:%02d:%02d.%03d", data.hour, data.minute, data.second, data.millisecond);
  sprintf(full_string, "RACER %d - %s", racer_number, data_string);
  Serial.println(full_string);
  printer.print(full_string);
  Serial.println("Done Printing");
}

void print_racer_data_to_sd(int racer_number, TimeResult data) {
  char filename[20];
  char full_string[25];
  char data_string[25];
  sprintf(data_string, "%2d,%02d,%03d", (data.hour * 60) + data.minute, data.second, data.millisecond);
  Serial.println("data_string");
  Serial.println(data_string);
  Serial.println("racer_number");
  Serial.println(racer_number);
  sprintf(full_string, "%d,%s", racer_number, data_string);
  
  build_race_filename(filename);
  sd.writeFile(filename, full_string);  
  // temporary
  printer.print(full_string);
  Serial.println(full_string);
}

void clear_previous_entry() {
  char filename[20];
  char message[20];
  build_race_filename(filename);

  sprintf(message, "CLEAR_PREVIOUS");
  Serial.println("Clear previous entry");
  printer.print(message);
  sd.writeFile(filename, message);
}

void print_filename() {
  char filename[20];
  build_race_filename(filename);
  printer.print(filename);
  printer.feed();
}
