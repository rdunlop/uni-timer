// Persistent Configuration
#include "uni_config.h"
#include <string.h>

#include "uni_sd.h"
extern UniSd sd;

UniConfig::UniConfig()
{

}

void UniConfig::setup() {
  // read Config
  if (!readConfig()) {
    _loadedFromDefault = true;
    // Default Config, as the config file is not found
    _config.start = true;
    _config.difficulty = 0;
    _config.up = true;
    _config.race_number = 0;
    _config.bib_number_length = 3;
    _config.start_line_countdown = false;
    _config.finish_line_spacing = 500;
    _config.mode = 1;
    _config.radio_enabled = false;
    _config.radio_id = 0;
    _config.radio_target_id = 0;
  } else {
    _loadedFromDefault = false;
  }
}

// Return true if the config exists on disk
bool UniConfig::loadedFromDefault() {
  return _loadedFromDefault;
}

char *UniConfig::filename() {
  snprintf(_config.filename, FILENAME_MAX_LENGTH, "/race_%s_%s_%s_%d.txt",
    _config.difficulty == 0 ? "Beginner" : _config.difficulty == 1 ? "Advanced" : "Expert",
    _config.up ? "Up" : "Down",
    _config.start ? "Start" : "Finish",
    _config.race_number);

  return _config.filename;
}

bool UniConfig::radioEnabled() {
  return _config.radio_enabled;
}

uint8_t UniConfig::radioID() {
  return _config.radio_id;
}

uint8_t UniConfig::radioTargetID() {
  return _config.radio_target_id;
}

void UniConfig::toggleRadioEnabled() {
  _config.radio_enabled = !_config.radio_enabled;
}

void UniConfig::incrementRadioID() {
  _config.radio_id += 1;
  if (_config.radio_id > MAX_RADIO_ID) {
    _config.radio_id = 0;
  }
}

void UniConfig::incrementRadioTargetID() {
  _config.radio_target_id += 1;
  if (_config.radio_target_id > MAX_RADIO_ID) {
    _config.radio_target_id = 0;
  }
}

// Start/difficulty/up/race_number
void UniConfig::toggle_start() {
  _config.start = !_config.start;
}

void UniConfig::increase_difficulty() {
  _config.difficulty = (_config.difficulty + 1) % 3;
}

void UniConfig::toggle_up() {
  _config.up = !_config.up;
}
void UniConfig::increment_race_number() {
  _config.race_number = (_config.race_number + 1) % 10;
}

bool UniConfig::get_start() {
  return _config.start;
}
int UniConfig::get_difficulty() {
  return _config.difficulty;
}
bool UniConfig::get_up() {
  return _config.up;
}
int UniConfig::get_race_number() {
  return _config.race_number;
}

// bib_number_length
void UniConfig::toggle_bib_number_length() {
  if (_config.bib_number_length == 3) {
    _config.bib_number_length = 4;
  } else {
    _config.bib_number_length = 3;
  }
}

int UniConfig::get_bib_number_length() {
  return _config.bib_number_length;
}

// start_line_countdown
void UniConfig::toggle_start_line_countdown() {
  _config.start_line_countdown = !_config.start_line_countdown;
}

bool UniConfig::get_start_line_countdown() {
  return _config.start_line_countdown;
}

// finish_line_spacing
void UniConfig::reset_finish_line_spacing() {
  _config.finish_line_spacing = 500;
}
void UniConfig::increment_finish_line_spacing(int ms) {
  _config.finish_line_spacing = (_config.finish_line_spacing + ms) % 1000;
}

int UniConfig::get_finish_line_spacing() {
  return _config.finish_line_spacing;
}

// mode
int UniConfig::mode() {
  return _config.mode;
}

void UniConfig::setMode(int mode) {
  _config.mode = mode;
  writeConfig();
}

/* ******************* PRIVATE METHODS ******************* */
// Return true if the string starts with prefix
bool UniConfig::prefix(const char *str, const char *prefix)
{
  return strncmp(prefix, str, strlen(prefix)) == 0;
}

// Return the value after the prefix
char *UniConfig::value(const char *str, const char *prefix)
{
  char *result = (char *)str;
  return result + strlen(prefix);
}

/* TODO, Refactor this into a condensed format?
[
  [&_config.start, "START:", "bool"],
  [&_config.difficulty, "DIFFICULTY:", "int"],
  etc
]
*/

// Read the config, return true on success
#define MAX_CONFIG_STRING 200
bool UniConfig::readConfig() {
  char data_string[MAX_CONFIG_STRING];
  if (sd.readConfig(data_string, MAX_CONFIG_STRING)) {
    char *token;
    token = strtok(data_string, "\n");
    while(token != NULL) {
      if (prefix(token, "START:")) {
        _config.start = atoi(value(token, "START:")) == 1;
      } else if (prefix(token, "DIFF:")) {
        _config.difficulty = atoi(value(token, "DIFF:"));
      } else if (prefix(token, "UP:")) {
        _config.up = atoi(value(token, "UP:")) == 1;
      } else if (prefix(token, "RACE:")) {
        _config.race_number = atoi(value(token, "RACE:"));
      } else if (prefix(token, "BIB_DIGITS:")) {
        _config.bib_number_length = atoi(value(token, "BIB_DIGITS:"));
      } else if (prefix(token, "COUNTDOWN:")) {
        _config.start_line_countdown = atoi(value(token, "COUNTDOWN:")) == 1;
      } else if (prefix(token, "SPACING:")) {
        _config.finish_line_spacing = atoi(value(token, "SPACING:"));
      } else if (prefix(token, "MODE:")) {
        _config.mode = atoi(value(token, "MODE:"));
      } else if (prefix(token, "RADIO_ENABLED:")) {
        _config.radio_enabled = atoi(value(token, "RADIO_ENABLED:")) == 1;
      } else if (prefix(token, "RADIO_ID:")) {
        _config.radio_id = atoi(value(token, "RADIO_ID:"));
      } else if (prefix(token, "RADIO_TARGET_ID:")) {
        _config.radio_target_id = atoi(value(token, "RADIO_TARGET_ID:"));
      }
      Serial.println("Got Config: ");
      Serial.println(token);
      token = strtok(NULL, "\n"); // Next token
    }
    return true;
  } else {
    Serial.println("Config file not found");
    return false;
  }
}

// Writes the configuration to the SD Card
// the format is:
// config_name|configuration value
bool UniConfig::writeConfig() {
  char data_string[MAX_CONFIG_STRING];
  snprintf(data_string, MAX_CONFIG_STRING,
    "%s%d\n"
    "%s%d\n"
    "%s%d\n"
    "%s%d\n"
    "%s%d\n"
    "%s%d\n"
    "%s%d\n"
    "%s%d\n"
    "%s%d\n"
    "%s%d\n"
    "%s%d\n",
    "START:", _config.start ? 1 : 0,
    "DIFF:", _config.difficulty,
    "UP:", _config.up ? 1 : 0,
    "RACE:", _config.race_number,
    "BIB_DIGITS:", _config.bib_number_length,
    "COUNTDOWN:", _config.start_line_countdown ? 1 : 0,
    "SPACING:", _config.finish_line_spacing,
    "MODE:", _config.mode,
    "RADIO_ENABLED:", _config.radio_enabled ? 1 : 0,
    "RADIO_ID:", _config.radio_id,
    "RADIO_TARGET_ID:", _config.radio_target_id
    );
  if (sd.writeConfig(data_string)) {
    Serial.println("Write File");
    Serial.println(data_string);
    return true;
  } else {
    Serial.println("Failed to write config file");
    return false;
  }
}
