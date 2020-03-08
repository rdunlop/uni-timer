// BUZZER
#include "uni_buzzer.h"
#include <Arduino.h>

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

// Beep for 1 second
void UniBuzzer::beep() {
  int channel = 0; // share this globally?
  ledcWriteTone(channel,800);
  _buzzerOn = true;
  _buzzerEndTime = millis() + 1000;
  
//  tone(_output, 1000, 100);
}

// Check to see if we should disable the buzzer
void UniBuzzer::checkBeep() {
  int channel = 0; // share this globally?
  if (!_buzzerOn) { return; }
  if (_buzzerEndTime < millis()) {
    ledcWriteTone(channel, 0);
    _buzzerOn = false;
  }
}
void UniBuzzer::success() {
  beep();
//  tone(_output, 1000, 100); // should be SUCCESS music
}
