// BUZZER
#include "uni_buzzer.h"
#include <Arduino.h>
#include "event_queue.h"

// typedef struct {
//   unsigned long startTime;
//   unsigned long endTime;
//   void (*callbackAtEventStart)();
// } BuzzerEntry;

// Because there is no support for scheduling
// future events on this processor, we have a
// simple scheduling system


UniBuzzer::UniBuzzer(int output)
{
  num_scheduled = 0;
  _output = output;
}

void UniBuzzer::setup() {
  _channel = 0;
  int frequency = 1E5;
  int resolution = 8;
  pinMode(_output, OUTPUT);
  ledcSetup(_channel, frequency, resolution);
  ledcAttachPin(_output, _channel);
  _buzzerOn = false;
}

void UniBuzzer::loop() {
  bool buzzerShouldBeOn = false;
  removeExpiredEntries();
  // Determine if ANY of the entries are activating the buzzer
  for (int i = 0; i < num_scheduled; i++) {
    BuzzerEntry entry = scheduled_entries[i];
    if (activeEntry(entry)) {
      buzzerShouldBeOn = true;
      if (!entry.callbackCalled && entry.callbackAtEntryStart != NULL) {
        Serial.println("calling callback function");
        entry.callbackCalled = true
        // entry.callbackAtEntryStart()
      }
    }
  }
  if (_buzzerOn != buzzerShouldBeOn) {
    push_event(EVT_BUZZER_CHANGE, buzzerShouldBeOn ? "1" : "0");

    if (buzzerShouldBeOn) {
      ledcWriteTone(_channel, 800);
      _buzzerOn = true;
    } else {
      ledcWriteTone(_channel, 0);
      _buzzerOn = false;
    }
  }
}

// remove entries which have passed
void UniBuzzer::removeExpiredEntries() {
  for (int i = 0; i < num_scheduled; i++) {
    BuzzerEntry entry = scheduled_entries[i];
    if (entry.endTime < millis()) {
      // copy the last entry onto the current entry
      memcpy(&entry, &scheduled_entries[num_scheduled - 1], sizeof(BuzzerEntry))
      num_scheduled--;
      // restart our search from 0 by setting the iterator to -1
      i = -1;
    }
  }
}

// return true if the entry is active
bool UniBuzzer::activeEntry(BuzzerEntry entry) {
  unsigned long now = millis();
  return (entry.startTime <= now) && (now <= entry.endTime);
}

// Beep for 1 second (or shorter)
void UniBuzzer::beep(int duration) {
  scheduleBeep(millis(), millis() + duration);
}

bool UniBuzzer::schedule(unsigned long start, unsigned long end) {
  if (num_scheduled >= MAX_SCHEDULED_BEEPS) {
    Serial.println("Too many scheduled beeps");
    return false;
  }
  BuzzerEntry next_beep = scheduled_entries[num_scheduled];
  next_entry.startTime = start;
  next_entry.endTime = end;
  next_entry.callbackAtEventStart = NULL;
  next_entry.callbackCalled = false;
  num_scheduled++;

  return true;
}

// Beep beep
void UniBuzzer::success() {
  unsigned long now = millis();

  schedule(now, now + 300);
  schedule(now + 600, now + 900);
}

// BEEEEEp, bep
void UniBuzzer::error() {
  unsigned long now = millis();
  schedule(now, now + 1000);
  schedule(now + 1100, now + 1200);
}

// beeep beeep
void UniBuzzer::warning() {
  unsigned long now = millis();

  schedule(now, now + 400);
  schedule(now + 800, now + 1200);
}

// callback function to be called when the countdown
// finishes
void cb() {
  char result[EVT_MAX_STR_LEN];
  TimeResult output;
  currentTime(&output);
  format_time_result(&output, result, EVT_MAX_STR_LEN);
  push_event(EVT_TIMER_COUNTDOWN_FINISHED, result);
}

// beep, beep, bEEEP
void UniBuzzer::countdown() {
  unsigned long now = millis();

  schedule(now       , now + 1000);
  schedule(now + 2000, now + 3000);
  schedule(now + 4000, now + 6000, cb);
}
