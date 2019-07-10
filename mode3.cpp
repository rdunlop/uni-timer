#include "uni_sensor.h"
#include "uni_buzzer.h"
#include "modes.h"

extern UniBuzzer buzzer;
extern UniSensor sensor;

//### Mode 3 - Sensor Tuning
//
//- When the Sensor beam is crossed, no noise. When the sensor is not-crossed, beep continuously.
void mode3_loop() {
  if (!sensor.blocked()) {
    buzzer.beep();
  }
}
