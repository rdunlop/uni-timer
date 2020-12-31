// Persistent Configuration
#include "uni_config.h"

#include "uni_sd.h"

extern UniSd sd;
#define CONFIG_FILENAME "/config.txt"

UniConfig::UniConfig()
{
  // read Config
  // readConfig(&_config);
}

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
bool UniConfig::readConfig(Config *config) {
  int max_config_string = 100;
  char data_string[max_config_string];
  if (sd.readFile(CONFIG_FILENAME, data_string, max_config_string)) {
    char *token;
    token = strtok(data_string, "\n");
    while(token != NULL) {
      if (prefix(token, "FILENAME:")) {
        strcpy(config->filename, value(token, "FILENAME:"));
      } else if (prefix(token, "MODE:")) {
        config->mode = atoi(value(token, "MODE:"));
      }
      Serial.println("Got Config: ");
      Serial.println(token);
      token = strtok(NULL, "\n"); // Next token
    }
    return true;
  } else {
    return false;
  }
}

// Writes the configuration to the SD Card
// the format is:
// config_name|configuration value
bool UniConfig::writeConfig(Config *config) {
  int max_config_string = 100;
  char data_string[max_config_string];
  snprintf(data_string, max_config_string,
    "%s%s\n%s%d\n",
    "FILENAME:", config->filename,
    "MODE:", config->mode
    );
  if (sd.writeFile(CONFIG_FILENAME, data_string)) {
    Serial.println("Write File");
    Serial.println(data_string);
    return true;
  } else {
    Serial.println("Failed to write config file");
    return false;
  }
}
