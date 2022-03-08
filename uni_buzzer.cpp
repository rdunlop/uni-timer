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
void UniBuzzer::success() {
  tone(_output, 1000, 100); // should be SUCCESS music
}

void UniBuzzer::failure() {
  tone(_output, 800, 100); // should be Failure music
}
