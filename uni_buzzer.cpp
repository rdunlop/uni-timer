// BUZZER
#include "uni_buzzer.h"
#include <Arduino.h>
#include "event_queue.h"
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
    BuzzerEntry *entry = &scheduled_entries[i];
    if (activeEntry(entry)) {
      buzzerShouldBeOn = true;
      if (!entry->callbackCalled && entry->callbackAtEventStart != NULL) {
        entry->callbackCalled = true;
        entry->callbackAtEventStart();
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

void UniBuzzer::clear() {
  ledcWriteTone(_channel, 0);
  _buzzerOn = false;
  num_scheduled = 0;
}

// remove entries which have passed
void UniBuzzer::removeExpiredEntries() {
  for (int i = 0; i < num_scheduled; i++) {
    BuzzerEntry *entry = &scheduled_entries[i];
    if (entry->endTime < millis()) {
      // copy the last entry onto the current entry, if there is more than 1
      memcpy(entry, &scheduled_entries[num_scheduled - 1], sizeof(BuzzerEntry));
      // Serial.print("removed ");
      // Serial.println(num_scheduled);
      num_scheduled--;
      // restart our search from 0 by setting the iterator to -1
      i = -1;
    }
  }
}

// return true if the entry is active
bool UniBuzzer::activeEntry(BuzzerEntry *entry) {
  unsigned long now = millis();
  return (entry->startTime <= now) && (now <= entry->endTime);
}

// Beep for 1 second (or shorter)
void UniBuzzer::beep(int duration) {
  schedule(millis(), millis() + duration);
}

bool UniBuzzer::schedule(unsigned long start, unsigned long end, void (*interrupt_handler)()) {
  if (num_scheduled >= MAX_SCHEDULED_BEEPS) {
    // Serial.println("Too many scheduled beeps");
    return false;
  }
  BuzzerEntry *next_entry = &scheduled_entries[num_scheduled];
  next_entry->startTime = start;
  next_entry->endTime = end;
  next_entry->callbackAtEventStart = interrupt_handler;
  next_entry->callbackCalled = false;
  num_scheduled++;
  // Serial.println("Scheduled beep");

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

// beep, beep, bEEEP
// calls the interrupt handler when the 3rd pulse starts
void UniBuzzer::countdown(void (*interrupt_handler)()) {
  unsigned long now = millis();

  schedule(now       , now + 1000);
  schedule(now + 2000, now + 3000);
  schedule(now + 4000, now + 6000, interrupt_handler);
}
