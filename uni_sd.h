#ifndef UNI_SD_H
#define UNI_SD_H
#include <SPI.h>
//#include "SdFat.h"
#include <SD.h>

class UniSd
{
  public:
    UniSd(int);
    void setup();
    void loop();
    bool status();
    void writeFile(char *filename, char *text);
    void readFile(char *filename);
  private:
    int _cs;
    boolean _status;
    void printDirectory(File, int);
    void printCard();
    File myFile;
};

#endif
