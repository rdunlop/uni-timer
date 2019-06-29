// - SD
#include "uni_sd.h"

UniSd::UniSd(int cs, int mosi, int miso, int clk)
{
  _cs = cs;
  _mosi = mosi;
  _miso = miso;
  _clk = clk;
}

void UniSd::setup() {

  _status = SD.begin(_cs);
  if (!_status) {
    Serial.println("SD initialization failed!");
    return;
  }
  Serial.println("SD initialization done.");
}

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
    Serial.print("Writing to ");
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
