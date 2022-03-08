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
    bool writeFile(const char *filename, char *text);
    bool readFile(const char *filename, char *result, int max_result);
  private:
    int _cs;
    bool _status;
    SdFat SD;
    File myFile;
};

#endif
