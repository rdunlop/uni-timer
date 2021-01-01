// Persistent Configuration
#include "uni_config.h"
#include <string.h>

#ifdef ENABLE_SD
#include "uni_sd.h"
extern UniSd sd;
#endif

#define CONFIG_FILENAME "/config.txt"

UniConfig::UniConfig()
{
  // read Config
  if (!readConfig()) {
    _loadedFromDefault = true;
    // Default Config, as the config file is not found
    _config.mode = 1;
    strncpy(_config.filename, "/results.txt", 40);
  } else {
    _loadedFromDefault = false;
  }
}

// Return true if the config exists on disk
bool UniConfig::loadedFromDefault() {
  return _loadedFromDefault;
}

char *UniConfig::filename() {
  return _config.filename;
}

void UniConfig::setFilename(char *filename) {
  strcpy(_config.filename, filename);
  writeConfig();
}

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

// Read the config, return true on success
bool UniConfig::readConfig() {
#ifdef ENABLE_SD
  int max_config_string = 100;
  char data_string[max_config_string];
  if (sd.readFile(CONFIG_FILENAME, data_string, max_config_string)) {
    char *token;
    token = strtok(data_string, "\n");
    while(token != NULL) {
      if (prefix(token, "FILENAME:")) {
        strcpy(_config.filename, value(token, "FILENAME:"));
      } else if (prefix(token, "MODE:")) {
        _config.mode = atoi(value(token, "MODE:"));
      }
      Serial.println("Got Config: ");
      Serial.println(token);
      token = strtok(NULL, "\n"); // Next token
    }
    return true;
  } else {
    return false;
  }
#else
  return false;
#endif
}

// Writes the configuration to the SD Card
// the format is:
// config_name|configuration value
bool UniConfig::writeConfig() {
#ifdef ENABLE_SD
  int max_config_string = 100;
  char data_string[max_config_string];
  snprintf(data_string, max_config_string,
    "%s%s\n%s%d\n",
    "FILENAME:", _config.filename,
    "MODE:", _config.mode
    );
  if (sd.writeFile(CONFIG_FILENAME, data_string)) {
    Serial.println("Write File");
    Serial.println(data_string);
    return true;
  } else {
    Serial.println("Failed to write config file");
    return false;
  }
#else
  return true;
#endif
}
