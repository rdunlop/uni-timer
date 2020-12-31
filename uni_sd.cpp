// - SD
#include "uni_sd.h"

UniSd::UniSd(int cs)
{
  _cs = cs;
}

void UniSd::setup() {

  _status = SD.begin(_cs);
  if (status()) {
    Serial.println("SD initialization OK.");
  } else {
    Serial.println("SD initialization failed!");
  }
  printCardContents();
}

void UniSd::printCardContents() {
  File root = SD.open("/");
  printDirectory(root, 0);
  root.close();
}

void UniSd::printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}

// Return true on success
// returns 1 on success
// returns 0 on failure
bool UniSd::status() {
  return _status;
//  writeFile("testfile.txt", "hEllo Robin");
//  writeFile("testfile.txt", "Goodbye Robin");
//  readFile("testfile.txt");
}

bool UniSd::writeFile(char *filename, char *text) {
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
    Serial.print("WRITE: error opening ");
    Serial.println(filename);
    return false;
  }
}

bool UniSd::readFile(char *filename, char *result, int max_result) {
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
    Serial.print("READ: error opening ");
    Serial.println(filename);
    return false;
  }
}
