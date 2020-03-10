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
    void success();
    void checkBeep();
  private:
    int _output;
    bool _buzzerOn;
    unsigned long _buzzerEndTime;
};

#endif
