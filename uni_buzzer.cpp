// BUZZER
#include "uni_buzzer.h"

UniBuzzer::UniBuzzer(int output)
{
  _output = output;
}

void UniBuzzer::setup() {
  pinMode(_output, OUTPUT);
}

void UniBuzzer::beep() {
  tone(_output, 1000, 100);
}
void UniBuzzer::success() {
  tone(_output, 1000, 100); // should be SUCCESS music
}
