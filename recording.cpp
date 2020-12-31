#include "uni_sd.h"
#include "recording.h"
#include "event_queue.h"
#include "uni_buzzer.h"

extern UniSd sd;
extern UniBuzzer buzzer;

int _racer_number = 0;

// Add a new digit to the current racer number
void store_racer_number(int racer_number) {
  _racer_number = racer_number;
  Serial.print("Racer #: ");
}

// Methods
void clear_racer_number() {
  _racer_number = 0;
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

// Config _config;

// Config *getConfig() {
//   return &_config;
// }

// void set_filename(const char *filename) {
//   snprintf(_config.filename, 40, filename);
// }
char *filename() {
  return "HELLO.txt";
}

void publish_time_recorded(int racer_number, char *data) {
  char data_string[EVT_MAX_STR_LEN];
  snprintf(data_string, EVT_MAX_STR_LEN, "%d,%s", racer_number, data);
  Serial.println("Publish Time");
  Serial.println(data_string);

  if (sd.writeFile(filename(), data_string)) {

  } else {
    buzzer.error();
  }

  push_event(EVT_TIME_STORED, data_string);
}

void print_racer_data_to_printer(int racer_number, TimeResult data) {
  #define MAX_RACER_DATA 35
  char full_string[MAX_RACER_DATA];
  char data_string[MAX_RACER_DATA];
  snprintf(data_string, MAX_RACER_DATA, "%02d:%02d:%02d.%03d", data.hour, data.minute, data.second, data.millisecond);
  snprintf(full_string, MAX_RACER_DATA, "RACER %d - %s", racer_number, data_string);
  Serial.println(full_string);
//  printer.print(full_string);
  Serial.println("Done Printing");
}

void print_racer_data_to_sd(int racer_number, TimeResult data) {
#define FILENAME_LENGTH 35
  char filename[FILENAME_LENGTH];
  char full_string[FILENAME_LENGTH];
  char data_string[FILENAME_LENGTH];
  snprintf(data_string, FILENAME_LENGTH, "%2d,%02d,%03d", (data.hour * 60) + data.minute, data.second, data.millisecond);
  Serial.println("data_string");
  Serial.println(data_string);
  Serial.println("racer_number");
  Serial.println(racer_number);
  snprintf(full_string, FILENAME_LENGTH, "%d,%s", racer_number, data_string);

  build_race_filename(filename, FILENAME_LENGTH);
  sd.writeFile(filename, full_string);
  // temporary
//  printer.print(full_string);
  Serial.println(full_string);
}

void clear_previous_entry() {
  #define MAX_FILENAME 35
  #define MAX_MESSAGE 20
  char filename[MAX_FILENAME];
  char message[MAX_MESSAGE];
  build_race_filename(filename, MAX_FILENAME);

  snprintf(message, MAX_MESSAGE, "CLEAR_PREVIOUS");
  Serial.println("Clear previous entry");
//  printer.print(message);
  sd.writeFile(filename, message);
}

void print_filename() {
  char filename[MAX_FILENAME];
  build_race_filename(filename, MAX_FILENAME);
//  printer.print(filename);
//  printer.feed();
}
