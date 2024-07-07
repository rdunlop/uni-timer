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
TimeResult recentResult[RECENT_RESULT_COUNT];
int recentRacer[RECENT_RESULT_COUNT];

// Add a new digit to the current racer number
void store_racer_number() {
  Serial.println("Storing Racer number");
  char last_key_pressed = keypad.lastKeyPressed();
  _racer_number = (_racer_number * 10) + keypad.intFromChar(last_key_pressed);
  Serial.print("Racer #: ");
  Serial.println(_racer_number);
  display.showNumber(_racer_number);
  sd.logCurrentRacerNumber(_racer_number);
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
void format_string(int racer_number, TimeResult data, bool fault, char *message, const int max_message) {
  snprintf(message, max_message, "%d,,%02d,%02d,%03d,%d",
    racer_number,
    (data.hour * 60) + data.minute, data.second, data.millisecond,
    fault ? 1 : 0
  );
}

bool print_racer_data_to_sd(int racer_number, TimeResult data, bool fault) {
#define FILENAME_LENGTH 35
  char filename[FILENAME_LENGTH];
  char full_string[FILENAME_LENGTH];

  format_string(racer_number, data, fault, full_string, FILENAME_LENGTH);

  #define REC_LINE_LENGTH 55
  char log_string[REC_LINE_LENGTH];
  snprintf(log_string, REC_LINE_LENGTH, "recording: %s", full_string);
  log(log_string);

  // Store result for review on the system as desired
  for (int i = RECENT_RESULT_COUNT - 1; i > 0; i--) {
    // copy result 8 to result 9,
    // copy result 7 to result 8, etc.
    memcpy(&recentResult[i], &recentResult[i - 1], sizeof(TimeResult));
    recentRacer[i] = recentRacer[i - 1];
  }
  // store result in slot 0
  memcpy(&recentResult[0], &data, sizeof(TimeResult));
  recentRacer[0] = racer_number;

  strlcpy(filename, config.filename(), FILENAME_LENGTH);
  if (sd.writeFile(filename, full_string)) {
    return true;
  } else {
    // Error writing to SD
    Serial.println("Error writing to SD");
    display.sdBad();
    return false;
  }
}

void print_data_to_log(TimeResult data, bool fault) {
  #define LINE_LENGTH 35
  char data_string[LINE_LENGTH];
  snprintf(data_string, LINE_LENGTH, "sensor: %2d,%02d,%02d,%03d,%d", data.hour, data.minute, data.second, data.millisecond, fault);
  log(data_string);
}

void clear_previous_entry() {
  #define MAX_FILENAME 35
  #define MAX_MESSAGE 20
  char filename[MAX_FILENAME];
  char message[MAX_MESSAGE];
  strlcpy(filename, config.filename(), MAX_FILENAME);

  snprintf(message, MAX_MESSAGE, "CLEAR_PREVIOUS");
  Serial.println("Clear previous entry");
  sd.writeFile(filename, message);
  log("Clear Previous entry");
}


void log(const char *message) {
  sd.log(message);
}
