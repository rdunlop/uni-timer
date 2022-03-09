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
  return _status;
//  writeFile("testfile.txt", "hEllo Robin");
//  writeFile("testfile.txt", "Goodbye Robin");
//  readFile("testfile.txt");
}

// append the given text to the file, as well as finish with a newline character (ie: println)
bool UniSd::writeFile(const char *filename, char *text) {
// open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open(filename, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing: ");
    Serial.println(text);
    Serial.print(" to ");
    Serial.println(filename);
    myFile.println(text);
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
  return sd.remove(filename);
}
