#ifndef UNI_GPS_H
#define UNI_GPS_H
#include <TinyGPS.h>

class UniGps
{
  static void pps_interrupt(); // registered interrupt handler with the system
  static UniGps * instance0_; // for use by interrupt handler
  
  public:
    UniGps(int pps_signal_input);
    void setup();
    void loop();
    void printPeriodically();
    int getDateTime(int *, int *);
  private:
    volatile unsigned long pps_start_ms;
    bool newData;
    uint32_t last_gps_print_time;
    int _pps_signal_input;
    TinyGPS gps;
    
    void handle_interrupt();
    void printGPS();
    void printGPSDate();
};

#endif
