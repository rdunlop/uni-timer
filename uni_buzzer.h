#ifndef UNI_BUZZER_H
#define UNI_BUZZER_H
#include "RTClib.h"

class UniBuzzer
{
  public:
    UniBuzzer(int output);
    void setup();
    void loop();
    void beep(int duration = 1000);
    void beep_next();
    void success();
    void warning();
    void error();
    void checkBeep();
    void countdown();
  private:
    int _output;
    bool _buzzerOn;
    unsigned long _buzzerEndTime;
    unsigned long _buzzerStartTime;
    // scheduled buzzings
    uint8_t num_scheduled;
    int scheduled_entries[10];
};

#endif
