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
    void test();
    void writeFile(char *filename, char *text);
    void readFile(char *filename);
  private:
    int _cs, _mosi, _miso, _clk;
    SdFat SD;
    File myFile;
};

#endif
