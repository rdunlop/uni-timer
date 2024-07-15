// BUZZER
#include "uni_buzzer.h"
#include <Arduino.h>

// #define MUTED true

UniBuzzer::UniBuzzer(int output)
{
  _output = output;
}

void UniBuzzer::setup() {
  pinMode(_output, OUTPUT);
  Serial.println("Buzzer Done init");
}

void UniBuzzer::beep() {
#ifdef MUTED
  tone(_output, 100, 100);
#else
  tone(_output, 1000, 100);
#endif
}

// beep when doing countdown
void UniBuzzer::pre_beep() {
#ifdef MUTED
  tone(_output, 100, 100);
#else
  tone(_output, 466, 500);
#endif
}

void UniBuzzer::start_beep() {
#ifdef MUTED
  tone(_output, 100, 100);
#else
  tone(_output, 932, 1000);
#endif
}

void UniBuzzer::success() {
#ifdef MUTED
  tone(_output, 100, 100);
#else
  tone(_output, 1000, 100); // should be SUCCESS music
#endif
}

void UniBuzzer::failure() {
#ifdef MUTED
  tone(_output, 100, 100);
#else
  tone(_output, 466, 200); // should be Failure music
  delay(200);
  tone(_output, 233, 200); // should be Failure music
#endif
}

void UniBuzzer::fault() {
#ifdef MUTED
  tone(_output, 100, 100);
#else
  tone(_output, 466, 200); // should be Failure music
#endif
}
