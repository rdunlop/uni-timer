#ifndef UNI_RTC_H
#define UNI_RTR_H
#include "RTClib.h"

class UniRtc
{
  static void sqw_interrupt(); // registered interrupt handler with the system
  static UniRtc * instance0_; // for use by interrupt handler
  
  public:
    UniRtc(int sqw_signal_input);
    void setup();
    void loop();
  private:
    volatile byte rtc_interrupt_flag;
    volatile unsigned long rtc_start_ms;
    uint32_t last_rtc_print_time;
    int _sqw_signal_input;
    RTC_DS3231 rtc;
    
    void handle_interrupt();
    void print();
};

#endif
