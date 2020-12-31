#ifndef UNI_BUZZER_H
#define UNI_BUZZER_H
#include "RTClib.h"
#define MAX_SCHEDULED_BEEPS 10

typedef struct {
  unsigned long startTime;
  unsigned long endTime;
  bool callbackCalled;
  void (*callbackAtEventStart)();
} BuzzerEntry;

class UniBuzzer
{
  public:
    UniBuzzer(int output);
    void setup();
    void loop();
    void beep(int duration = 1000);
    void clear();

    void success();
    void warning();
    void error();
    void countdown(void (*interrupt_handler)());
  private:
    bool schedule(unsigned long start, unsigned long end, void (*interrupt_handler)() = NULL);
    void removeExpiredEntries();
    bool activeEntry(BuzzerEntry *);
    int _output;
    bool _buzzerOn;
    int _channel;
    // scheduled buzzings
    uint8_t num_scheduled;
    BuzzerEntry scheduled_entries[MAX_SCHEDULED_BEEPS];
};

#endif
