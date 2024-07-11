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

  uint32_t setup_start = millis();
  // Setup Internal SD Card
  _internal_ok = initInternalSD();
  if (_internal_ok) {
    Serial.println("Int SD initialization OK.");
  } else {
    Serial.println("Int SD Initialization Failed!");
  }
  Serial.println("Internal init took");
  Serial.println(millis() - setup_start);
  setup_start = millis();

  // Setup External SD Card
  // returns 1 on success
  // returns 0 on failure
  _external_ok = initExternalSD();
  if (_external_ok) {
    Serial.println("SD initialization OK.");
  } else {
    Serial.println("SD initialization failed!");
  }
  Serial.println("External init took");
  Serial.println(millis() - setup_start);
  setup_start = millis();

  if (status()) {
    Serial.println("BOTH SD write-test passed");
  } else {
    Serial.println("Failed performing write-test");
  }
  Serial.println("Status took");
  Serial.println(millis() - setup_start);
  setup_start = millis();
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
  return sd.begin(SdioConfig(FIFO_SDIO));
}

bool UniSd::initExternalSD() {
  return sd.begin(SdSpiConfig(_cs, SHARED_SPI, SD_SCK_MHZ(16)));
}


// Write to a file, and read back, to ensure SD card is working
// return true on success
bool UniSd::testWriteExternal() {
  if (!_external_ok) { return true; }

  if (!initExternalSD()) {
    Serial.println("SD initialization failed!");
    return false;
  } else {
    Serial.println("ext SD init");
  }
  FsFile testFile = sd.open("testfilee.txt", O_CREAT | O_WRITE | O_AT_END);
  int result = testFile.println("testing Write 1");
  if (result > 0) {
    testFile.close();
    return true;
  } else {
    Serial.println("EResult");
    Serial.println(result);
    return false;
  }
}

// return true on success
bool UniSd::testWriteInternal() {
  if (!_internal_ok) { return true; }
  if (!initInternalSD()) {
    Serial.println("int SD initialization failed!");
    return false;
  } else {
    Serial.println("int SD init");
  }
  FsFile testFile = sd.open("inttestfilei.txt", O_CREAT | O_WRITE | O_AT_END);
  int result = testFile.println("testing Write 1");
  if (result > 0) {
    testFile.close();
    return true;
  } else {
    Serial.println("IResult");
    Serial.println(result);
    return false;
  }
}

// append the given text to the file, as well as finish with a newline character (ie: println)
bool UniSd::writeFile(const char *filename, const char *text) {
  bool success = true;

  if (_internal_ok) {
    if (!testWriteInternal()) {
      success = false;
    } else {
      if (!writeFilePrivate(filename, text)) {
        success = false;
      }
    }
  }

  if (_external_ok) {
    // EXTERNAL SD
    if (!testWriteExternal()) {
      success = false;
    } else {
      if (!writeFilePrivate(filename, text)) {
        success = false;
      }
    }
  }

  return success;
}

bool UniSd::writeFilePrivate(const char *filename, const char *text) {
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  FsFile myFile = sd.open(filename, O_CREAT | O_WRITE | O_AT_END);

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
  if (_internal_ok) {
    initInternalSD();
  } else {
    initExternalSD();
  }

  FsFile myFile = sd.open(CONFIG_FILENAME, O_READ);

  // retry if failure
  // this may happen if the SD card was removed between uses
  if (!myFile) {
    setup();
    myFile = sd.open(CONFIG_FILENAME, O_READ);
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
  if (_internal_ok) {
    initInternalSD();
    sd.remove(CONFIG_FILENAME);
  }
  if (_external_ok) {
    initExternalSD();
    sd.remove(CONFIG_FILENAME);
  }
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
