#ifndef UNI_SD_H
#define UNI_SD_H
#include <SPI.h>
#include <SD.h>

class UniSd
{
  public:
    UniSd(int);
    // api
    void setup();
    bool status();
    bool readConfig(char *result, int max_result); // reads from internal SD file
    bool writeConfig(const char *config_string);
    bool log(const char *message);

    // new methods
    void logCurrentRacerNumber(int); // writes to the log file
    void logRaceResult(); // writes to the data file

    // internal
    bool writeFile(const char *filename, const char *text);
    bool writeFilePrivate(const char *filename, const char *text);
    bool testWriteInternal();
    bool testWriteExternal();
  private:
    int _cs;
    bool _status;
    bool initInternalSD();
    bool initExternalSD();
};

#endif
