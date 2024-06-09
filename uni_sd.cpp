// - SD
// Handles both internal and external SD Card management
#include "uni_sd.h"
#include "recording.h" // for currentTime and TimeResult

#define CONFIG_FILENAME "config.txt"
#define LOG_FILENAME "log.txt"

UniSd::UniSd(int cs)
{
  _cs = cs;
}

// ****************************************
// API - Functions used by other parts of the program in order
// to interact with the SD cards sub-system
// ****************************************

// Initialize the SD cards, ensure that they are both functional
void UniSd::setup() {

  // Setup Internal SD Card
  bool internal_status = initInternalSD();
  if (internal_status) {
    Serial.println("Int SD initialization OK.");
  } else {
    Serial.println("Int SD Initialization Failed!");
  }

  // Setup External SD Card
  // returns 1 on success
  // returns 0 on failure
  bool external_status = initExternalSD();
  if (external_status) {
    Serial.println("SD initialization OK.");
  } else {
    Serial.println("SD initialization failed!");
  }
  status();

  _status = internal_status && external_status;
}

// Return true when both SD cards are fully functioning
bool UniSd::status() {
  // further test that we can write and read
  return testWriteInternal() && testWriteExternal();
}

// ****************************************
// INTERNAL Functions
// ****************************************
bool UniSd::initInternalSD() {
  return SD.begin(BUILTIN_SDCARD);
}

bool UniSd::initExternalSD() {
  return SD.begin(_cs);
}


// Write to a file, and read back, to ensure SD card is working
// return true on success
bool UniSd::testWriteExternal() {
  if (!initExternalSD()) {
    Serial.println("SD initialization failed!");
  }
  File testFile = SD.open("testfile.txt", FILE_WRITE);
  int result = testFile.println("testing Write 1");
  if (result > 0) {
    testFile.close();
    return true;
  } else {
    return false;
  }
}

// return true on success
bool UniSd::testWriteInternal() {
  if (!initInternalSD()) {
    Serial.println("int SD initialization failed!");
  }
  File testFile = SD.open("inttestfile.txt", FILE_WRITE);
  int result = testFile.println("testing Write 1");
  if (result > 0) {
    testFile.close();
    return true;
  } else {
    return false;
  }
}

// append the given text to the file, as well as finish with a newline character (ie: println)
bool UniSd::writeFile(const char *filename, const char *text) {
  if (!testWriteInternal()) {
    return false;
  }
  bool success = true;

  if (!writeFilePrivate(filename, text)) {
    success = false;
  }

  // EXTERNAL SD
  if (!testWriteExternal()) {
    return false;
  }

  if (!writeFilePrivate(filename, text)) {
    success = false;
  }

  return success;
}

bool UniSd::writeFilePrivate(const char *filename, const char *text) {
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File myFile = SD.open(filename, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to ");
    Serial.print(filename);
    Serial.print(": ");
    Serial.println(text);
    if (!myFile.println(text)) {
      myFile.close();
      return false;
    }
    // close the file:
    myFile.close();
    return true;
  } else {
    // if the file didn't open, print an error:
    Serial.print("error opening ");
    Serial.println(filename);
    return false;
  }
}

bool UniSd::readConfig(char *result, int max_result) {
  // re-open the file for reading:
  initInternalSD();
  File myFile = SD.open(CONFIG_FILENAME);

  // retry if failure
  // this may happen if the SD card was removed between uses
  if (!myFile) {
    setup();
    myFile = SD.open(CONFIG_FILENAME);
  }

  if (myFile) {
    Serial.println(CONFIG_FILENAME);

    // read from the file until there's nothing else in it:
    int current_position = 0;
    while (myFile.available()) {
      char character = myFile.read();
      if (current_position < max_result) {
        result[current_position++] = character;
      }

      Serial.write(character);
    }
    // close the file:
    myFile.close();
    return true;
  } else {
    // if the file didn't open, print an error:
    Serial.print("error opening ");
    Serial.println(CONFIG_FILENAME);
    return false;
  }
}

bool UniSd::writeConfig(const char *config_string) {
  initInternalSD();
  SD.remove(CONFIG_FILENAME);
  return writeFile(CONFIG_FILENAME, config_string);
}

void UniSd::logCurrentRacerNumber(int racer_number) {
  char str[40];
  snprintf(str, 40, "Current Racer Number: %d", racer_number);
  log(str);
}

// write log message to both SD cards
// return true on success
bool UniSd::log(const char *message) {
  Serial.print("LOG: ");
  Serial.println(message);

  TimeResult data;
  currentTime(&data);
  char sz[100];
  snprintf(sz, 100, "%02d:%02d:%02d.%03d: %s",
      data.hour, data.minute, data.second, data.millisecond, message);
  Serial.println(sz);

  return writeFile(LOG_FILENAME, sz);
}
