#ifndef UNI_GPS_H
#define UNI_GPS_H
#include <TinyGPS.h>

typedef struct {
  byte hour;
  byte minute;
  byte second;
  int millisecond;
} TimeResult;

class UniGps
{
  public:
    UniGps(int pps_signal_input);
    void setup(void (*interrupt_handler)());
    void readData();
    void printPeriodically();
    bool current_time(TimeResult *, unsigned long current_micros);
    void printGPSDate();
    bool lock();
    unsigned long charactersReceived();
    bool synchronizeClocks(unsigned long current_micros);
  private:
    bool newData;
    uint32_t last_gps_print_time;
    unsigned long _last_pps_micros;
    unsigned long _last_gps_time_in_seconds;
    int _pps_signal_input;
    TinyGPS gps;
    void printGPS();
    
};

#endif
