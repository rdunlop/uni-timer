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
    UniGps(int pps_signal_input) :
      _last_hour(0),
      _pps_signal_input(pps_signal_input)
      {};
    void setup(void (*interrupt_handler)());
    void readData();
    bool detected();
    void printPeriodically();
    bool current_time(TimeResult *, unsigned long current_millis);
    void printGPSDate();
    bool lock();
    unsigned long charactersReceived();
    bool synchronizeClocks(unsigned long current_millis);
  private:
    bool newData;
    uint32_t last_gps_print_time;
    volatile unsigned long _last_pps_millis;
    volatile unsigned long _last_gps_time_in_seconds;
    byte _last_hour; // to prevent the 'hour' from wrapping around when GPS date advances
    int _pps_signal_input;
    TinyGPS gps;
    void printGPS();
    
};

#endif
