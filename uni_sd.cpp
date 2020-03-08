// - SD
#include "uni_sd.h"

UniSd::UniSd(int cs)
{
  _cs = cs;
}

void UniSd::setup() {
  // returns 1 on success
  // returns 0 on failure
  _status = SD.begin(5);
  if (status()) {
    Serial.println("SD initialization OK.");  
  } else {
    Serial.println("SD initialization failed!");
  }
  printCard();
}

void UniSd::printCard() {
  File root = SD.open("/");
  printDirectory(root, 0);
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
bool UniSd::status() {
  return _status;
//  writeFile("testfile.txt", "hEllo Robin");
//  writeFile("testfile.txt", "Goodbye Robin");
//  readFile("testfile.txt");
}

void UniSd::writeFile(char *filename, char *text) {
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
  } else {
    // if the file didn't open, print an error:
    Serial.print("error opening ");
    Serial.println(filename);
  }
}

void UniSd::readFile(char *filename) {
  // re-open the file for reading:
  myFile = SD.open(filename);
  if (myFile) {
    Serial.println(filename);

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.print("error opening ");
    Serial.println(filename);
  }
}
