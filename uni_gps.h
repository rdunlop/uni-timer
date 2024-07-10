#ifndef UNI_GPS_H
#define UNI_GPS_H
// #define USE_ADA
#define USE_TINY
#ifdef USE_TINY
#include <TinyGPS.h>
#endif
#ifdef USE_ADA
#include <Adafruit_GPS.h>
#endif

typedef struct {
  byte hour;
  byte minute;
  byte second;
  int millisecond;
} TimeResult;

class UniGps
{
  public:
    #ifdef USE_ADA
    UniGps(int pps_signal_input, HardwareSerial *serial) :
      _last_hour(0),
      _pps_signal_input(pps_signal_input),
      GPS(serial),
      _chars_processed(0)
      {};
    #endif
    #ifdef USE_TINY
    UniGps(int pps_signal_input) :
    _last_hour(0),
    _pps_signal_input(pps_signal_input)
      {};
    #endif
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
    #ifdef USE_TINY
    TinyGPS gps;
    #endif
    #ifdef USE_ADA
    Adafruit_GPS GPS;
    unsigned long _chars_processed;
    #endif
    void printGPS();
    
};

#endif
