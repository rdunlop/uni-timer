// SENSOR
#include "Arduino.h"
#include "uni_sensor.h"
#include "event_queue.h"

UniSensor::UniSensor(int input)
{
  _input = input;
  pinMode(_input, INPUT);
  _last_sensor = blocked();
}

void UniSensor::setupInterruptHandler(void (*interrupt_handler)()) {
  _interrupt_handler = interrupt_handler;
}


void UniSensor::loop() {
  bool sensor_value = blocked();
  if (_last_sensor != sensor_value) {
     push_event(EVT_SENSOR_CHANGE, sensor_value ? "1" : "0");
     Serial.println("blocked");
     _last_sensor = sensor_value;
  }
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
