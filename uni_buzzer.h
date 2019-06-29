#ifndef UNI_BUZZER_H
#define UNI_BUZZER_H
#include "RTClib.h"

class UniBuzzer
{
  public:
    UniBuzzer(int output);
    void setup();
    void loop();
    void beep();
  private:
    int _output;
};

#endif