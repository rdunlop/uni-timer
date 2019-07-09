// SENSOR
#include "Arduino.h"
#include "uni_sensor.h"

UniSensor::UniSensor(int input)
{
  _input = input;
}

UniSensor * UniSensor::instance0_;

void UniSensor::setup() {
  pinMode(_input, INPUT);
  instance0_ = this;
}

bool UniSensor::blocked() {
  return digitalRead(_input);
}

bool UniSensor::blocked_via_interrupt() {
  return _interrupt_micros != 0;
}

void UniSensor::sensor_interrupt() {
  instance0_->handle_interrupt();
}
void UniSensor::handle_interrupt() {
  _interrupt_micros = micros();
  Serial.println("INTERRUPTED");
  Serial.println(_interrupt_micros);
}

unsigned long UniSensor::interrupt_micros() {
  return _interrupt_micros;
}

void UniSensor::clear_interrupt_micros() {
  _interrupt_micros = 0;
}

void UniSensor::attach_interrupt() {
  attachInterrupt(digitalPinToInterrupt(_input), sensor_interrupt, RISING);
}

void UniSensor::detach_interrupt() {
  detachInterrupt(digitalPinToInterrupt(_input));
}
