// SENSOR
#include "Arduino.h"
#include "uni_sensor.h"

UniSensor::UniSensor(int input)
{
  _input = input;
}

void UniSensor::setup(void (*interrupt_handler)()) {
  pinMode(_input, INPUT);
  _interrupt_handler = interrupt_handler;
}

bool UniSensor::blocked() {
  return digitalRead(_input);
}

void UniSensor::attach_interrupt() {
  attachInterrupt(digitalPinToInterrupt(_input), _interrupt_handler, RISING);
}

void UniSensor::detach_interrupt() {
  detachInterrupt(digitalPinToInterrupt(_input));
}
