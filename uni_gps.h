#ifndef UNI_GPS_H
#define UNI_GPS_H
#include <TinyGPS.h>

class UniGps
{
  public:
    UniGps(int pps_signal_input);
    void setup(void (*interrupt_handler)());
    void readData();
    void printPeriodically();
    int getHourMinuteSecond(int *, int *, int *);
    bool current_time(byte *hour, byte *minute, byte *second);
    void printGPSDate();
    bool lock();
  private:
    bool newData;
    uint32_t last_gps_print_time;
    int _pps_signal_input;
    TinyGPS gps;
    void printGPS();
    
};

#endif
