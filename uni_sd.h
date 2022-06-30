#ifndef UNI_SD_H
#define UNI_SD_H
#include <SPI.h>
#include "SdFat.h"

class UniSd
{
  public:
    UniSd(int);
    void setup();
    void loop();
    bool status();
    bool clearFile(const char *filename);
    bool writeFile(const char *filename, char *text);
    bool readFile(const char *filename, char *result, int max_result);
    bool testWrite();
  private:
    int _cs;
    bool _status;
    SdFat SD;
    File myFile;
};

#endif
