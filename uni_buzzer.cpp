// BUZZER
#include "uni_buzzer.h"
#include <Arduino.h>
#include "event_queue.h"

// Because there is no support for scheduling
// future events on this processor, we have a
// simple scheduling system
uint8_t num_scheduled = 0;
#define MAX_SCHEDULED_BEEPS 10
int scheduled_entries[MAX_SCHEDULED_BEEPS];

UniBuzzer::UniBuzzer(int output)
{
  _output = output;
}

void UniBuzzer::setup() {
  int channel = 0;
  int frequency = 1E5;
  int resolution = 8;
  pinMode(_output, OUTPUT);
  ledcSetup(channel, frequency, resolution);
  ledcAttachPin(_output, channel);
  _buzzerOn = false;
  _buzzerEndTime = 0;
}

// Beep for 1 second (or shorter)
void UniBuzzer::beep(int duration) {
  int channel = 0; // share this globally?
  ledcWriteTone(channel,800);
  _buzzerOn = true;
  _buzzerEndTime = millis() + duration;
  push_event(EVT_BUZZER_CHANGE, "1");
}

int pop_scheduled_time() {
  int next_time = scheduled_entries[0];
  for (int i = 1; i < num_scheduled; i++) {
    scheduled_entries[i - 1] = scheduled_entries[i];
  }
  num_scheduled--;

  return next_time;
}

// Beep for the duration of the next entry
void UniBuzzer::beep_next() {
  int duration = pop_scheduled_time();
  int channel = 0; // share this globally?
  ledcWriteTone(channel,800);
  _buzzerOn = true;
  _buzzerEndTime = millis() + duration;
  push_event(EVT_BUZZER_CHANGE, "1");
}


// Check to see if we should disable the buzzer, or enable the buzzer
void UniBuzzer::checkBeep() {
  int channel = 0; // share this globally?
  if (_buzzerOn) {
    if (_buzzerEndTime < millis()) {
      ledcWriteTone(channel, 0);
      _buzzerOn = false;
      push_event(EVT_BUZZER_CHANGE, "0");
      if (num_scheduled > 0) {
        _buzzerStartTime = millis() + pop_scheduled_time();
      } else {
        push_event(EVT_TIMER_COUNTDOWN_FINISHED, "1:2:3");
      }
    }
  } else {
    if (_buzzerStartTime == 0) { return; }
    if (_buzzerStartTime < millis()) {
      _buzzerStartTime = 0;
      beep_next();
    }
  }
}

void schedule(int entries[], const uint8_t num_entries) {
  if (num_entries >= MAX_SCHEDULED_BEEPS) {
    num_scheduled = 0;
    return;
  }
  for(int i = 0; i < num_entries; i++) {
    scheduled_entries[i] = entries[i];
  }
  num_scheduled = num_entries;
}

void UniBuzzer::success() {
  int buzzes[] = {300, 300, 300, 300};
  schedule(buzzes, 4);
  beep_next();
}

void UniBuzzer::error() {
  int buzzes[] = {1000, 100, 100, 100};
  schedule(buzzes, 4);
  beep_next();
}

void UniBuzzer::warning() {
  int buzzes[] = {400, 400, 400, 400};
  schedule(buzzes, 4);
  beep_next();
}

void UniBuzzer::countdown() {
  int buzzes[] = {1000, 1000, 1000, 1000, 2000, 1000};
  schedule(buzzes, 6);
  beep_next();
}
