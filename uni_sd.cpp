// - SD
#include "uni_sd.h"

UniSd::UniSd(int cs)
{
  _cs = cs;
}

void UniSd::setup() {
  // returns 1 on success
  // returns 0 on failure
  _status = SD.begin(_cs);
  if (status()) {
    Serial.println("SD initialization OK.");  
    return;
  }
  Serial.println("SD initialization failed!");
  
}

// Return true on success
bool UniSd::status() {
  // further test that we can write and read
  return testWrite();
}

// Write to a file, and read back, to ensure SD card is working
// return true on success
bool UniSd::testWrite() {
  File testFile;
  bool newstatus = SD.begin(_cs);
  testFile = SD.open("testfile.txt", FILE_WRITE);
  int result = testFile.println("testing Write");
  if (result > 0) {
    testFile.close();
    return true;
  } else {
    return false;
  }
}

// append the given text to the file, as well as finish with a newline character (ie: println)
bool UniSd::writeFile(const char *filename, char *text) {
  if (!testWrite()) {
    return false;
  }

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open(filename, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing: ");
    Serial.println(text);
    Serial.print(" to ");
    Serial.println(filename);
    if (!myFile.println(text)) {
      myFile.close();
      return false;
    }
    // close the file:
    myFile.close();
    Serial.println("done.");
    return true;
  } else {
    // if the file didn't open, print an error:
    Serial.print("error opening ");
    Serial.println(filename);
    return false;
  }
}

bool UniSd::readFile(const char *filename, char *result, int max_result) {
  // re-open the file for reading:
  myFile = SD.open(filename);

  // retry if failure
  // this may happen if the SD card was removed between uses
  if (!myFile) {
    setup();
    myFile = SD.open(filename);
  }

  if (myFile) {
    Serial.println(filename);

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
    Serial.println(filename);
    return false;
  }
}

bool UniSd::clearFile(const char *filename) {
  return SD.remove(filename);
}
