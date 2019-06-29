#ifndef UNI_SD_H
#define UNI_SD_H
#include <SPI.h>
#include "SdFat.h"

class UniSd
{
  public:
    UniSd(int, int, int, int);
    void setup();
    void loop();
    bool status();
    void writeFile(char *filename, char *text);
    void readFile(char *filename);
  private:
    int _cs, _mosi, _miso, _clk;
    bool _status;
    SdFat SD;
    File myFile;
};

#endif
