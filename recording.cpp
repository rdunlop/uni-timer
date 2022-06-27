#include "uni_display.h"
#include "uni_keypad.h"
#include "uni_sd.h"
#include "recording.h"
#include "uni_config.h"

extern UniDisplay display;
extern UniKeypad keypad;
extern UniSd sd;
extern UniConfig config;

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
  log("Clear Racer Number");
}

int racer_number() {
  return _racer_number;
}

// Is the racer number already maximum-digits long?
// if so, another digit will be "too long"
bool maximum_digits_racer_number() {
  if (config.get_bib_number_length() == 3) {
    return racer_number() > 99;
  } else {
    return racer_number() > 999;
  }
}



// **((((((((( NEW FILE )))))))))))))))))

void print_racer_data_to_sd(int racer_number, TimeResult data, bool fault) {
#define FILENAME_LENGTH 35
  char filename[FILENAME_LENGTH];
  char full_string[FILENAME_LENGTH];
  char data_string[FILENAME_LENGTH];
  snprintf(data_string, FILENAME_LENGTH, "%2d,%02d,%03d", (data.hour * 60) + data.minute, data.second, data.millisecond);
  Serial.println("data_string");
  Serial.println(data_string);
  Serial.println("racer_number");
  Serial.println(racer_number);
  if (fault) {
    snprintf(full_string, FILENAME_LENGTH, "%d,%s,FAULT", racer_number, data_string);
  } else {
    snprintf(full_string, FILENAME_LENGTH, "%d,%s", racer_number, data_string);
  }

  strncpy(filename, config.filename(), FILENAME_LENGTH);
  sd.writeFile(filename, full_string);
  // temporary
  Serial.println(full_string);
}

void clear_previous_entry() {
  #define MAX_FILENAME 35
  #define MAX_MESSAGE 20
  char filename[MAX_FILENAME];
  char message[MAX_MESSAGE];
  strncpy(filename, config.filename(), MAX_FILENAME);

  snprintf(message, MAX_MESSAGE, "CLEAR_PREVIOUS");
  Serial.println("Clear previous entry");
  sd.writeFile(filename, message);
  log("Clear Previous entry");
}

#define LOG_FILE "log.txt"
void log(char *message) {
  sd.writeFile(LOG_FILE, message);
}
