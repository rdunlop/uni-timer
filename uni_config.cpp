// Persistent Configuration
#include "uni_config.h"

#include "uni_sd.h"

extern UniSd sd;
#define CONFIG_FILENAME "config.txt"

UniConfig::UniConfig()
{
  // read Config
  // readConfig(&_config);
}

// Read the config, return true on success
bool UniConfig::readConfig(Config *config) {
  int max_config_string = 100;
  char data_string[max_config_string];
  if (sd.readFile(CONFIG_FILENAME, data_string, max_config_string)) {
    char *token;
    token = strtok(data_string, "|");
    while(token != NULL) {
      Serial.println("Got Config: ");
      Serial.println(token);
      token = strtok(NULL, "|"); // Next token
    }
  }
  // memcpy(config, &_config);
  return true;
}

// Writes the configuration to the SD Card
// the format is:
// config_name|configuration value
bool UniConfig::writeConfig(Config *config) {
  int max_config_string = 100;
  char data_string[max_config_string];
  snprintf(data_string, max_config_string,
    "%s|%s",
    "FILENAME", config->filename
    );
  if (sd.writeFile(CONFIG_FILENAME, data_string)) {
    return true;
  } else {
    return false;
  }
}
