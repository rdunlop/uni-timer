// BUZZER
#include "uni_buzzer.h"
#include <Arduino.h>

UniBuzzer::UniBuzzer(int output)
{
  _output = output;
}

void UniBuzzer::setup() {
  pinMode(_output, OUTPUT);
  Serial.println("Buzzer Done init");
}

void UniBuzzer::beep() {
  tone(_output, 1000, 100);
}

// beep when doing countdown
void UniBuzzer::pre_beep() {
  tone(_output, 466, 500);
}

void UniBuzzer::start_beep() {
  tone(_output, 932, 1000);
}

void UniBuzzer::success() {
  tone(_output, 1000, 100); // should be SUCCESS music
}

void UniBuzzer::failure() {
  tone(_output, 466, 200); // should be Failure music
  delay(200);
  tone(_output, 233, 200); // should be Failure music
}

void UniBuzzer::fault() {
  tone(_output, 466, 200); // should be Failure music
}
