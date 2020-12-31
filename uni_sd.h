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
    bool writeFile(char *filename, char *text);
    bool readFile(char *filename, char *result, int max_result);
  private:
    int _cs;
    bool _status;
    void printDirectory(File, int);
    void printCardContents();
    File myFile;
};

#endif
